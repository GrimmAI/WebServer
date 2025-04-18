import os
import json
import torch
import numpy as np
from PIL import Image
from datetime import datetime
from cn_clip.clip import load_from_name
from tqdm import tqdm

# 配置参数
image_folder = "/home/mazhaomeng/cpp/WebServer/src/test/statics/image"  # 图片文件夹路径
output_bin = "/home/mazhaomeng/cpp/WebServer/Index/features.bin"   # 特征向量输出文件
output_json = "/home/mazhaomeng/cpp/WebServer/Index/features_meta.json"  # 元数据文件
supported_exts = [".jpg", ".jpeg", ".png", ".webp"]  # 支持的图片格式

# 加载中文CLIP模型
device = "cuda" if torch.cuda.is_available() else "cpu"
model, preprocess = load_from_name("ViT-B-16", device=device, download_root='/home/mazhaomeng/cpp/WebServer/model')
model.eval()

# 遍历文件夹获取图片路径
image_paths = []
for root, _, files in os.walk(image_folder):
    for file in files:
        ext = os.path.splitext(file)[1].lower()
        if ext in supported_exts:
            image_paths.append(os.path.join(root, file))

# 提取特征并保存
features = []
index_mapping = {}
failed_files = []

with torch.no_grad():
    for idx, img_path in tqdm(enumerate(image_paths)):
        try:
            # 加载和预处理图片
            image = Image.open(img_path).convert("RGB")
            image_tensor = preprocess(image).unsqueeze(0).to(device)
            
            # 提取特征并归一化
            feature = model.encode_image(image_tensor)
            feature /= feature.norm(dim=-1, keepdim=True)
            
            # 保存特征和路径映射
            features.append(feature.cpu().numpy().astype(np.float32))
            index_mapping[str(idx)] = os.path.abspath(img_path)  # 存储绝对路径

        except Exception as e:
            print(f"处理失败: {img_path} - {str(e)}")
            failed_files.append(img_path)
            continue

# 生成元数据
metadata = {
    "model": "ViT-B-16",  # 使用的模型名称
    "num_images": len(features),  # 成功处理的图片数量
    "feature_dim": features[0].shape[1] if features else 0,  # 特征维度（如512）
    "timestamp": datetime.now().isoformat(),  # 生成时间戳
    "file_structure": {
        "bin_file": output_bin,
        "data_type": "float32",
        "binary_layout": "num_images x feature_dim"
    },
    "failed_files": failed_files  # 记录失败文件
}

# 保存数据
if features:
    # 合并所有特征为单个numpy数组
    features_np = np.concatenate(features, axis=0)
    
    # 保存二进制特征文件
    with open(output_bin, "wb") as f:
        f.write(features_np.tobytes())

    # 保存元数据和索引
    metadata["index_mapping"] = index_mapping  # 将路径映射添加到元数据
    with open(output_json, "w", encoding='utf-8') as f:
        json.dump(metadata, f, indent=2, ensure_ascii=False)
        
    print(f"成功保存 {len(features)} 张图片特征到 {output_bin}")
    print(f"元数据文件: {output_json}")
else:
    print("未提取到有效特征")