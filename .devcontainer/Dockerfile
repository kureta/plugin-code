FROM nvcr.io/nvidia/pytorch:22.08-py3

# Create directory for custom libs
ENV MYHOME /home/devuser
RUN mkdir -p $MYHOME/lib

# Get libtorch cuda
ADD https://download.pytorch.org/libtorch/cu116/libtorch-cxx11-abi-shared-with-deps-1.12.1%2Bcu116.zip $MYHOME/lib/libtorch.zip
RUN unzip -q $MYHOME/lib/libtorch.zip -d $MYHOME/lib/
RUN rm $MYHOME/lib/libtorch.zip
# Remove conflicting libs
RUN rm $MYHOME/lib/libtorch/lib/libcu*

# Get supercollider source
RUN git clone --recurse-submodules --branch=Version-3.12.2 https://github.com/supercollider/supercollider.git $MYHOME/lib/supercollider/

# Setup user
RUN addgroup --gid 1000 devuser
RUN adduser --disabled-password --gecos "" --uid 1000 --gid 1000 devuser
ENV HOME $MYHOME

# Change ownership of downloaded stuff
RUN chown -R devuser:devuser $HOME

# Change user
USER devuser
