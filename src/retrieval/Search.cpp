#include "Search.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <algorithm>
#include <unordered_map>

#define SHM_NAME "/cn_clip_shm"
#define SHM_SIZE (1024 * 1024)

Search::Search() {
    std::system("python3 /home/mazhaomeng/cpp/WebServer/src/utils/get_text_embedding.py &");
}

Search::~Search() {
    std::system("pkill -f /home/mazhaomeng/cpp/WebServer/src/utils/get_text_embedding.py");
}

float Search::CalcSim(std::vector<float> &a, std::vector<float> &b) {
    float res = 0;
    for (u_int32_t i = 0; i < a.size(); i++) {
        res += a[i] * b[i];
    }
    return res;
}

std::vector<float> Search::GetUserQueryTextEmbedding(std::string &text) {
    // 共享内存设置
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        throw std::runtime_error("shm_open failed");
    }

    // 内存映射
    void* shm_ptr = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        close(shm_fd);
        throw std::runtime_error("mmap failed");
    }

    // 写入文本数据
    size_t text_len = text.size() + 1; // 包含null终止符
    if (text_len > SHM_SIZE) {
        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        throw std::runtime_error("Text exceeds SHM capacity");
    }
    memcpy(shm_ptr, text.c_str(), text_len);

    // 信号文件路径
    const std::string req_file = "/home/mazhaomeng/cpp/WebServer/request.lock";
    const std::string done_file = "/home/mazhaomeng/cpp/WebServer/done.lock";
    const std::string output_bin = "/home/mazhaomeng/cpp/WebServer/text_embedding.bin";

    // 触发处理
    std::ofstream(req_file).close();
    
    // 等待处理完成
    while (!std::filesystem::exists(done_file)) {
        usleep(10000); // 10ms轮询间隔
    }

    // 读取结果
    std::ifstream bin_stream(output_bin, std::ios::binary);
    if (!bin_stream) {
        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        throw std::runtime_error("Failed to open result file");
    }

    std::vector<float> features(512);
    bin_stream.read(reinterpret_cast<char*>(features.data()), 512*sizeof(float));

    // 清理资源
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    remove(req_file.c_str());
    remove(done_file.c_str());
    remove(output_bin.c_str());

    return features;
}

std::vector<std::string> Search::GetTopKResults(std::vector<float> query_embedding, int top_k) {
    std::ifstream meta_file("/home/mazhaomeng/cpp/WebServer/Index/features_meta.json");
    if (!meta_file.is_open()) {
        std::cerr << "无法打开元数据文件" << std::endl;
        return {};
    }

    // 解析JSON
    Json::Value metadata;
    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, meta_file, &metadata, &errs)) {
        std::cerr << "JSON解析错误: " << errs << std::endl;
        return {};
    }

    // 提取并验证数值
    int num_vectors = metadata["num_images"].asInt();
    int feature_dim = metadata["feature_dim"].asInt();
    std::string bin_file_path = metadata["file_structure"]["bin_file"].asString();

    std::unordered_map<int, std::string> index_mapping;

    const Json::Value& mapping = metadata["index_mapping"];
    for (const auto& key : mapping.getMemberNames()) {
        int index = std::stoi(key);
        const Json::Value& value = mapping[key];
        index_mapping[index] = value.asString();
    }


    // 处理二进制文件路径
    std::filesystem::path abs_path = std::filesystem::absolute(bin_file_path);
    std::ifstream bin_file(abs_path, std::ios::binary);
    if (!bin_file.is_open()) {
        std::cerr << "无法打开二进制文件: " << abs_path << std::endl;
        return {};
    }

    // 验证文件大小
    bin_file.seekg(0, std::ios::end);
    size_t file_size = bin_file.tellg();
    bin_file.seekg(0, std::ios::beg);

    size_t expected_size = num_vectors * feature_dim * sizeof(float);
    if (file_size != expected_size) {
        std::cerr << "文件大小不匹配: 预期 " << expected_size 
                  << " 字节，实际 " << file_size << " 字节" << std::endl;
        return {};
    }

    // 读取数据
    std::vector<float> features(num_vectors * feature_dim);
    bin_file.read(reinterpret_cast<char*>(features.data()), file_size);
    if (!bin_file) {
        std::cerr << "读取二进制文件失败: 仅读取了 " << bin_file.gcount() << " 字节" << std::endl;
        return {};
    }

    std::vector<std::pair<float, int>> sims(num_vectors);

    for (int i = 0; i < num_vectors; ++i) {
        std::vector<float> feature(feature_dim);
        for (int j = 0; j < feature_dim; ++j) {
            feature[j] = features[i * feature_dim + j];
        }
        float sim = CalcSim(feature, query_embedding);
        sims[i] = std::make_pair(sim, i);
    }

    std::sort(sims.begin(), sims.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
        return a.first > b.first;
    });

    std::vector<std::string> res(top_k);

    for (int i = 0; i < top_k; i++) {
        // std::cout << sims[i].first << " idx: " << sims[i].second << std::endl;
        res[i] = index_mapping[sims[i].second];
    }
    return res;
}