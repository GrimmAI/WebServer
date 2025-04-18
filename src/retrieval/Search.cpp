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

#define SHM_SIZE 1024 * 1024

Search::Search() {

}

Search::~Search() {

}

float Search::CalcSim(std::vector<float> &a, std::vector<float> &b) {
    float res = 0;
    for (u_int32_t i = 0; i < a.size(); i++) {
        res += a[i] * b[i];
    }
    return res;
}

std::vector<float> Search::GetUserQueryTextEmbedding(std::string query_text) {
    // 创建共享内存
    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open failed");
        return {};
    }
    ftruncate(fd, SHM_SIZE);

    // 映射共享内存
    void* ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return {};
    }

    std::memcpy(ptr, query_text.c_str(), query_text.size() + 1);

    // 启动Python程序
    std::system("python3 /home/mazhaomeng/cpp/WebServer/src/utils/get_text_embedding.py &");

    // 等待Python程序处理完成
    while (true) {
        std::ifstream check_file("/home/mazhaomeng/cpp/WebServer/done.txt");
        if (check_file) {
            check_file.close();
            break;
        }
        sleep(1);
    }

    std::vector<float> res(512);
    // // 从共享内存读取特征向量
    // float* features = static_cast<float*>(ptr);
    // 计算特征向量的起始位置
    const float* features = reinterpret_cast<const float*>(
        static_cast<const char*>(ptr) + query_text.size()
    );

    // 假设特征向量的大小为512
    for (int i = 0; i < 512; ++i) {
        res[i] = features[i];
    }

    // 清理
    munmap(ptr, SHM_SIZE);
    close(fd);
    shm_unlink("/my_shm");

    // 删除通知文件
    unlink("/home/mazhaomeng/cpp/WebServer/done.txt");
    return res;
}

std::vector<std::string> Search::GetTopKResults(std::vector<float> query_embedding, int top_k) {
    for (u_int32_t i = 0; i < query_embedding.size(); i++) {
        std::cout << query_embedding[i] << ", ";
    }
    std::cout << std::endl;
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