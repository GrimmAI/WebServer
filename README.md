
### 将OpenCv动态库集成到系统的库目录中（可选）  
sudo apt update  
安装 OpenCV 核心库  
sudo apt install -y libopencv-dev  
sudo apt install -y libopencv-core-dev  
sudo apt install -y libopencv-imgproc-dev   
查看已安装的OpenCV版本  
pkg-config --modversion opencv4  
sudo apt install -y libjpeg-dev libpng-dev libtiff-dev  

### wsl启用gpu(可选)
在WSL中安装CUDA Toolkit
wget https://developer.download.nvidia.com/compute/cuda/12.6.3/local_installers/cuda_12.6.3_560.35.05_linux.run
sh cuda_12.6.3_560.35.05_linux.run
rm cuda_12.6.3_560.35.05_linux.run

设置环境变量：编辑~/.bashrc文件，添加以下行:  
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
source ~/.bashrc

安装CUDA 工具包  
sudo apt install nvidia-cuda-toolkit  

安装cuDNN
下载.deb到downloads文件夹下  
sudo dpkg -i cudnn-local-repo-ubuntu2204-9.8.0_1.0-1_amd64.deb

sudo cp /var/cudnn-local-repo-ubuntu2204-9.8.0/cudnn-local-*-keyring.gpg /usr/share/keyrings/
sudo apt update
sudo apt install -y libcudnn9-cuda-12 libcudnn9-dev-cuda-12

确保CUDA路径已添加到环境变量
echo "export LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH" >> ~/.bashrc
source ~/.bashrc


### 安装libtorch(cpu)







