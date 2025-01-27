
# OpenCL Project

First OpenCL project in order to acquire new skills.

## Vector Addition

TODO

## Matrix Multiplication

Work in Progress.

## Softmax Kernel

TODO

## Install

```sh
apt-get install intel-opencl-icd
apt-get install opencl-headers
apt-get install clinfo

ln -s libOpenCL.so.1 libOpenCL.so
apt-get install intel-gpu-tools
```

<!-- ocl-icd-dev ocl-icd-libopencl1 ocl-icd-opencl-dev -->

OpenCL consists of a set of headers and a shared object that is loaded at
runtime. An installable client driver (ICD) must be installed on the platform
for every class of vendor for which the runtime would need to support. That is,
for example, in order to support Nvidia devices on a Linux platform, the Nvidia
ICD would need to be installed such that the OpenCL runtime (the ICD loader)
would be able to locate the ICD for the vendor and redirect the calls
appropriately. The standard OpenCL header is used by the consumer application;
calls to each function are then proxied by the OpenCL runtime to the appropriate
driver using the ICD. Each vendor must implement each OpenCL call in their
driver.

The Apple, Nvidia, ROCm, RapidMind and Gallium3D implementations of OpenCL are
all based on the LLVM Compiler technology and use the Clang compiler as their
frontend.

- https://www.intel.com/content/www/us/en/developer/articles/tool/opencl-drivers.html#cpu-section
- https://www.intel.com/content/www/us/en/developer/articles/technical/intel-cpu-runtime-for-opencl-applications-with-sycl-support.html
- https://www.intel.com/content/www/us/en/docs/oneapi/installation-guide-linux/2023-0/apt.html#GUID-560A487B-1B5B-4406-BB93-22BC7B526BCD

```sh
# download the key to system keyring
wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \
  | gpg --dearmor | tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null

# add signed entry to apt sources and configure the APT client to use Intel repository:
echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" \
  | tee /etc/apt/sources.list.d/oneAPI.list

apt-get install intel-oneapi-runtime-opencl
```

## Sources

- https://en.wikipedia.org/wiki/OpenCL
- https://github.com/intel/compute-runtime?tab=readme-ov-file
- https://github.com/Oblomov/clinfo
- https://wiki.archlinux.org/title/GPGPU
