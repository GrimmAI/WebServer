import numpy as np
import mmap
import os
import time
import torch

import cn_clip.clip as clip
from cn_clip.clip import load_from_name, available_models

start_time = time.time()
device = "cuda" if torch.cuda.is_available() else "cpu"
model, _ = load_from_name("ViT-B-16", device=device, download_root='/home/mazhaomeng/cpp/WebServer/model')
model.eval()


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

text_token = clip.tokenize([text]).to(device)

with torch.no_grad():
    text_features = model.encode_text(text_token)
    # 对特征进行归一化，请使用归一化后的图文特征用于下游任务
    text_features /= text_features.norm(dim=-1, keepdim=True) 
# 模拟特征提取
features = text_features.cpu().numpy().astype(np.float32)
# print("features from py: ", features)

output_bin = "/home/mazhaomeng/cpp/WebServer/temp_text_embedding.bin"
with open(output_bin, "wb") as f:
    f.write(features.tobytes())

# # 写入共享内存
# text_bytes = text.encode('utf-8') + b'\x00'
# padding = (4 - (len(text_bytes) % 4)) % 4
# buf.write(text_bytes + b'\x00' * padding)
# buf.write(features.tobytes())

# 通知C++程序处理完成
with open("/home/mazhaomeng/cpp/WebServer/done.txt", "w") as f:
    f.write("done")

end_time = time.time()
print(end_time - start_time)
# 清理
buf.close()
os.close(fd)