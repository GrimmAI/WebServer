# get_text_embedding_service.py
import numpy as np
import mmap
import os
import time
import torch

import cn_clip.clip as clip
from cn_clip.clip import load_from_name

# 常驻模型加载
device = "cuda" if torch.cuda.is_available() else "cpu"
model, _ = load_from_name("ViT-B-16", device=device, download_root='/home/mazhaomeng/cpp/WebServer/model')
model.eval()

# 共享配置
SHM_NAME = "/dev/shm/cn_clip_shm"
SHM_SIZE = 1024 * 1024  # 1MB
REQUEST_FILE = "/home/mazhaomeng/cpp/WebServer/request.lock"
DONE_FILE = "/home/mazhaomeng/cpp/WebServer/done.lock"
OUTPUT_BIN = "/home/mazhaomeng/cpp/WebServer/text_embedding.bin"

class SharedMemoryManager:
    def __init__(self):
        # 创建或附加共享内存
        self.fd = os.open(SHM_NAME, os.O_RDWR | os.O_CREAT, 0o666)
        os.ftruncate(self.fd, SHM_SIZE)
        self.buf = mmap.mmap(self.fd, SHM_SIZE, mmap.MAP_SHARED, 
                           mmap.PROT_READ | mmap.PROT_WRITE)
        
    def __enter__(self):
        return self.buf
        
    def __exit__(self, *args):
        self.buf.close()
        os.close(self.fd)

def inference_service():
    with SharedMemoryManager() as shm:
        while True:
            # 等待请求信号
            while not os.path.exists(REQUEST_FILE):
                time.sleep(0.01)
                
            # 读取输入文本
            shm.seek(0)
            text = shm.read(shm.find(b'\x00')).decode('utf-8')
            
            # 特征提取
            with torch.no_grad():
                tokens = clip.tokenize([text]).to(device)
                features = model.encode_text(tokens)
                features /= features.norm(dim=-1, keepdim=True)
            
            # 保存结果
            features.cpu().numpy().astype(np.float32).tofile(OUTPUT_BIN)
            
            # 发送完成信号
            with open(DONE_FILE, 'w') as f:
                f.write(str(os.getpid()))
                
            # 清理请求
            os.remove(REQUEST_FILE)

if __name__ == "__main__":
    try:
        inference_service()
    finally:
        # 清理共享内存
        if os.path.exists(SHM_NAME):
            os.remove(SHM_NAME)