rm -rf ./build
mkdir build
cd build/
rm -rf *
cmake .. -DCMAKE_C_COMPILER=/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc \
     -DCMAKE_CXX_COMPILER=/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++ \
     -DCMAKE_Fortran_COMPILER=/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gfortran \
     -DCMAKE_ASM_COMPILER=/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc \
     -DCMAKE_SYSTEM_NAME=Generic \
     -DCMAKE_Fortran_COMPILER_WORKS=1 \
     -DCMAKE_C_COMPILER_WORKS=1 \
     -DCMAKE_CXX_COMPILER_WORKS=1

make -j$(nproc)