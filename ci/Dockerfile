FROM archlinux:latest

#Install dependencies including Node.js for CI runner actions
RUN pacman -Syu --noconfirm \
    clang lld libc++ clang-tools-extra \
    cmake ninja git \
    python python-pip \
    nodejs npm \
    libgl mesa \
    freetype2 fontconfig libxkbcommon \
    && pacman -Scc --noconfirm

#Verify clang-scan-deps and node exist (fail fast, avoid silent CI failures)
RUN clang-scan-deps --version && node --version

#Install Qt with aqt into /opt/Qt
RUN pip install --no-cache-dir --break-system-packages aqtinstall && \
    aqt install-qt linux desktop 6.12.0 linux_gcc_64 --outputdir /opt/Qt && \
    pip uninstall -y --break-system-packages aqtinstall

ENV CMAKE_PREFIX_PATH=/opt/Qt/6.12.0/gcc_64