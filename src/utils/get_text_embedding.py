import numpy as np
import mmap
import os
import time
import torch

import cn_clip.clip as clip
from cn_clip.clip import load_from_name, available_models

SHM_SIZE = 1024 * 1024  # 1MB
SHM_NAME = "/dev/shm/my_shm"  # 使用绝对路径

# 等待共享内存对象创建
while True:
    try:
        fd = os.open(SHM_NAME, os.O_RDWR)
        break
    except FileNotFoundError:
        time.sleep(1)

buf = mmap.mmap(fd, SHM_SIZE, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)

# 读取文本
text = buf.readline().decode('utf-8').strip()
print(f"Received text: {text}")

device = "cuda" if torch.cuda.is_available() else "cpu"
model, _ = load_from_name("ViT-B-16", device=device, download_root='/home/mazhaomeng/cpp/WebServer/model')
model.eval()
text_token = clip.tokenize([text]).to(device)

with torch.no_grad():
    text_features = model.encode_text(text_token)
    # 对特征进行归一化，请使用归一化后的图文特征用于下游任务
    text_features /= text_features.norm(dim=-1, keepdim=True) 
# 模拟特征提取
features = text_features.cpu().numpy().astype(np.float32)
print("features from py: ", features)

# print(features)
# 写入特征向量到共享内存
text_length = len(text) + 1
buf.seek(text_length)
buf.write(features.tobytes())

# 通知C++程序处理完成
with open("/home/mazhaomeng/cpp/WebServer/done.txt", "w") as f:
    f.write("done")

# 清理
buf.close()
os.close(fd)