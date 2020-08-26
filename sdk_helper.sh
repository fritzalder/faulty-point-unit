#!/bin/bash

# This file is modified from the install scripts of 
#  https://github.com/jovanbulck/sgx-step
# As the two install scripts are the only files we actuall need from
# that repository, we do not include it as a subproject but rather
# accept this minimal code duplication.

# This file is a complete helper to switch between
# two SDK versions. Use it as follows:
ALLOWED_INPUTS="[patched|2.8|vulnerable|2.7|help|install|install_sim|remove]"
# Example: ./sdk_helper.sh $ALLOWED_INPUTS
# install: Install the Intel SGX Driver + PSW + SDK of the major version 2.8 
#          system wide and also install the SGX SDK in version 2.7.1 locally.
# install_sim: If there is no Intel SGX available on the machine or if it is desired
#               to just run the simulation mode, use install_sim to only install the SDKs
# help: Print helper text to show paths and what is installed
# patched|2.8:    Switch to the SDK in version 2.8
# vulnerable|2.7: Switch to the SDK in version 2.7.1 (vulnerable to FPU attacks)

# Please note that you need to source the script when switching SDKs to be sure to inherit the environment variables!

# Define directory names
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SGX_REPOS=$BASE_DIR/"dependencies"
SGX_DRIVER=$SGX_REPOS/"linux-sgx-driver"
SGX_VULNERABLE=$SGX_REPOS/"sgx-sdk-2.7.1_vulnerable"
SGX_VULNERABLE_INSTALL=$SGX_REPOS/"sdk_2.7.1._installation"
SGX_PATCHED=$SGX_REPOS/"sgx-sdk-2.8_patched"
SGX_DEFAULT_LOCATION=/opt/intel

function print_paths_if_installed(){
    # Simply check whether the vulnerable install directory is empty to determine an install
    if [ "$(ls -A $SGX_VULNERABLE_INSTALL)" ]; then
        echo ""
        echo "Installed are the following dependencies for this artifact:"
        echo "Systemwide installations:"
        echo " - Linux SGX SDK v2.8.0"
        echo "   -   This Version patched the described vulnerability"
        echo "   -   Use it with"
        echo "   source /opt/intel/sgxsdk/environment"
        echo "Local installation:"
        echo " - Linux SGX SDK v2.7.1"
        echo "   -   This is the last major vulnerable SDK version"
        echo "   -   Installed at $SGX_VULNERABLE_INSTALL"
        echo "   -   Use it with"
        echo "   source $SGX_VULNERABLE_INSTALL/sgxsdk/environment"
        echo ""
        echo " If installed in hardware mode, we also installed the driver and psw:"
        echo " - Linux SGX Driver of 2.8"
        echo " - Linux PSW of 2.8"
        echo ""
        echo "You can utilize this script to switch between SDKs:"
        echo "source ./sdk_helper.sh $ALLOWED_INPUTS"
        echo "E.g. 'source ./sdk_helper.sh patched' to switch to SDK version 2.8"
        echo " and 'source ./sdk_helper.sh vulnerable to switch back to the vulnerable 2.7.1"
        echo ""
        echo "To uninstall the Intel SGX tools again, run ./sdk_helper.sh remove"
    else
        echo "We seem to not be installed yet."
    fi
}

function print_usage(){
    echo "Usage: sdk_helper $ALLOWED_INPUTS"
    echo " help           -- Show help plus additional information if already installed"
    echo " install        -- Install the SDKs and SGX driver"
    echo " install_sim    -- Install the SDKs but not driver"
    echo " remove         -- Remove all that was installed"
    echo " patched|2.8    -- Switch to the patched SDK"
    echo " vulnerable|2.7 -- Switch to the vulnerable SDK"
}

function print_helper_text() {
    echo "###########################################################"
    print_usage
    print_paths_if_installed
    echo "###########################################################"
}

# Define expected environment variables for each SDK version
ENV_SGX_SDK_PATCHED=$SGX_DEFAULT_LOCATION/sgxsdk
ENV_PATH_PATCHED=$ENV_SGX_SDK_PATCHED/bin:$ENV_SGX_SDK_PATCHED/bin/x64
ENV_PKG_CONFIG_PATH_PATCHED=$ENV_SGX_SDK_PATCHED/pkgconfig
ENV_LD_LIBRARY_PATH_PATCHED=$ENV_SGX_SDK_PATCHED/sdk_libs

ENV_SGX_SDK_VULNERABLE=$SGX_VULNERABLE_INSTALL/sgxsdk
ENV_PATH_VULNERABLE=$ENV_SGX_SDK_VULNERABLE/bin:$ENV_SGX_SDK_VULNERABLE/bin/x64
ENV_PKG_CONFIG_PATH_VULNERABLE=$ENV_SGX_SDK_VULNERABLE/pkgconfig
ENV_LD_LIBRARY_PATH_VULNERABLE=$ENV_SGX_SDK_VULNERABLE/sdk_libs

function print_environment_variables(){
    echo "Set environment variables as follows:"
    echo "PATH: $PATH"
    echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
    echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
}

function clear_environment_variables(){
    # Clear the Path from old SDK paths
        # First, remove any mention of $ENV_SGX_SDK_PATCHED or $ENV_SGX_SDK_VULNERABLE
    NEW_PATH=$(echo "$PATH"     | sed -e "s|$ENV_PATH_PATCHED||g")
    NEW_PATH=$(echo "$NEW_PATH" | sed -e "s|$ENV_PATH_VULNERABLE||g")

    # Clear PKG Config
    NEW_PKG_CONFIG=$(echo "$PKG_CONFIG_PATH" | sed -e "s|$ENV_PKG_CONFIG_PATH_PATCHED||g")
    NEW_PKG_CONFIG=$(echo "$NEW_PKG_CONFIG"  | sed -e "s|$ENV_PKG_CONFIG_PATH_VULNERABLE||g")

    # Clear LD Library Path
    NEW_LD_LIBRARY_PATH=$(echo "$LD_LIBRARY_PATH"     | sed -e "s|$ENV_LD_LIBRARY_PATH_PATCHED||g")
    NEW_LD_LIBRARY_PATH=$(echo "$NEW_LD_LIBRARY_PATH" | sed -e "s|$ENV_LD_LIBRARY_PATH_VULNERABLE||g")

}

function cleanup_environment_variables(){
    NEW_PATH=$(echo "$PATH" | sed -e "s|::*|:|g") # Reduce double colon to single colon
    NEW_PATH=$(echo "$NEW_PATH" | sed -e "s|:$||") # Remove colon at end of line
    NEW_PATH=$(echo "$NEW_PATH" | sed -e "s|^:||") # Remove colon at start of line
    NEW_PATH=$(echo "$NEW_PATH" | sed -e "s|\(\.:\)\+|.:|g") # Prevent duplication of .: in Path

    NEW_PKG_CONFIG=$(echo "$PKG_CONFIG_PATH"  | sed -e "s|::*|:|g")
    NEW_PKG_CONFIG=$(echo "$NEW_PKG_CONFIG"  | sed -e "s|:$||")
    NEW_PKG_CONFIG=$(echo "$NEW_PKG_CONFIG"  | sed -e "s|^:||")

    NEW_LD_LIBRARY_PATH=$(echo "$LD_LIBRARY_PATH" | sed -e "s|::*|:|g")
    NEW_LD_LIBRARY_PATH=$(echo "$NEW_LD_LIBRARY_PATH" | sed -e "s|:$||")
    NEW_LD_LIBRARY_PATH=$(echo "$NEW_LD_LIBRARY_PATH" | sed -e "s|^:||")

    export PATH=$NEW_PATH
    export PKG_CONFIG_PATH=$NEW_PKG_CONFIG
    export LD_LIBRARY_PATH=$NEW_LD_LIBRARY_PATH

}

function enable_patched(){
    # First, remove any other SDK paths
    clear_environment_variables

    # Then, enable patched SDK (same as SDK environment file)
    export SGX_SDK=$ENV_SGX_SDK_PATCHED
    export PATH=$NEW_PATH:$ENV_PATH_PATCHED
    export PKG_CONFIG_PATH=$NEW_PKG_CONFIG:$ENV_PKG_CONFIG_PATH_PATCHED
    if [ -z "$NEW_LD_LIBRARY_PATH" ]; then
        export LD_LIBRARY_PATH=$ENV_LD_LIBRARY_PATH_PATCHED
    else
        export LD_LIBRARY_PATH=$NEW_LD_LIBRARY_PATH:$ENV_LD_LIBRARY_PATH_PATCHED
    fi
    cleanup_environment_variables
    print_environment_variables
}

function enable_vulnerable(){
    # First, remove any other SDK paths
    clear_environment_variables

    # Then, enable vulnerable SDK (same as SDK environment file)
    export SGX_SDK=$ENV_SGX_SDK_VULNERABLE
    export PATH=$NEW_PATH:$ENV_PATH_VULNERABLE
    export PKG_CONFIG_PATH=$NEW_PKG_CONFIG:$ENV_PKG_CONFIG_PATH_VULNERABLE
    if [ -z "$NEW_LD_LIBRARY_PATH" ]; then
        export LD_LIBRARY_PATH=$ENV_LD_LIBRARY_PATH_VULNERABLE
    else
        export LD_LIBRARY_PATH=$NEW_LD_LIBRARY_PATH:$ENV_LD_LIBRARY_PATH_VULNERABLE
    fi

    print_environment_variables
}

function install_common(){
    # At the time of this artifact, there is an issue in newer Linux systems
    # that prevents proper driver usage and prevents mmap(PROT_EXEC) on the SGX device
    # A temporary workaround right now is to remount /dev with exec permissions 
    # To be future-proof, we limit this workaround to ubuntu 20.04
    if [ "$(lsb_release -sr)" = "20.04" ]; then
    echo "Remounting /dev due to issue in Ubuntu 20.04 with the SGX driver"
    sudo mount -o remount,exec /dev
    fi

    mkdir -p $SGX_VULNERABLE_INSTALL

    git submodule init
    git submodule update

    # ----------------------------------------------------------------------
    echo "[ installing prerequisites ]"
    sudo apt-get -yqq install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
    sudo apt-get -yqq install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
    sudo apt-get -yqq install python3 python3-pip python3-matplotlib

    # Boost program options for MNist example
    sudo apt-get -yqq install libboost-dev libboost-program-options-dev libboost-system-dev
    # tqdm for Python evaluation scripts
    sudo pip3 install tqdm matplotlib numpy

    # Compile the fpu_lib library
    make -C lib
}

function install_driver(){
    # ----------------------------------------------------------------------
    # Install current driver
    cd $SGX_DRIVER
    sudo apt-get -yqq install linux-headers-$(uname -r)
    make
    sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"    
    sudo cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"    
    sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"    
    sudo /sbin/depmod
    sudo /sbin/modprobe isgx

    echo "SGX driver succesfully installed"

}

function install_psw(){
    # ----------------------------------------------------------------------
    # Install PSW system wide
    echo "[ building 2.8 PSW ]"
    cd $SGX_PATCHED
    make psw_install_pkg

    echo "[ installing 2.8 PSW/SDK system-wide ]"
    cd linux/installer/bin/

    if [ -e /opt/intel/sgxpsw/uninstall.sh ]
    then
        sudo /opt/intel/sgxpsw/uninstall.sh
    fi
    sudo ./sgx_linux_x64_psw_*.bin

    echo "SGX PSW 2.8 succesfully installed!"


}

function install_vulnerable_sdk(){
    # ----------------------------------------------------------------------
    # Install vulnerable SDK locally
    echo "[ building vulnerable SDK version 2.7.1 ]"
    cd $SGX_VULNERABLE
    ./download_prebuilt.sh > /dev/null
    make -j`nproc` sdk_install_pkg > /dev/null

    echo "[ installing SDK 2.7.1 at $SGX_REPOS/ ]"
    cd linux/installer/bin/
sudo ./sgx_linux_x64_sdk_*.bin << EOF
no
$SGX_VULNERABLE_INSTALL
EOF
    cd $BASE_DIR

}

function install_patched_sdk(){
    # ----------------------------------------------------------------------
    # Install patched SDK system wide
    echo "[ building patched SDK version 2.8 ]"
    cd $SGX_PATCHED
    ./download_prebuilt.sh > /dev/null
    make -j`nproc` sdk_install_pkg > /dev/null

    echo "[ installing SDK at $SGX_REPOS/ ]"
    cd linux/installer/bin/
sudo ./sgx_linux_x64_sdk_*.bin << EOF
no
/opt/intel
EOF

    cd $BASE_DIR
}

function install_sdks(){
install_patched_sdk
install_vulnerable_sdk
}

if [ $# -ne 1 ]; then
    echo "Error: Expecting one argument! Usage: sdk_helper $ALLOWED_INPUTS"
    print_usage
    echo "Exiting."
    exit
fi


case $1 in
install)
echo "Installing all.."
set -e
install_common
install_driver
install_sdks
install_psw
enable_vulnerable
print_helper_text
;;
install_sim)
echo "Installing SDKs only for simulation mode.."
set -e
install_common
install_sdks
enable_vulnerable
print_helper_text
;;
install_patched)
echo "Installing only patched SDK for Travis simulation mode.."
set -e
install_common
install_patched_sdk
enable_vulnerable
print_helper_text
;;
install_vulnerable)
echo "Installing only vulnerable SDK for Travis simulation mode.."
set -e
install_common
install_vulnerable_sdk
enable_vulnerable
print_helper_text
;;
patched|2.8)
echo "Switching to patched SDK 2.8..."
enable_patched
echo "done."
echo "REMEMBER: Did you source this script as in 'source ./sdk_helper patched'? Otherwise the environment variables might be lost and not inherited by your shell. You can double check with env | grep sgx."
;;
vulnerable|2.7)
echo "Switching to vulnerable SDK 2.7.1..."
enable_vulnerable
echo "done."
echo "REMEMBER: Did you source this script as in 'source ./sdk_helper vulnerable'? Otherwise the environment variables might be lost and not inherited by your shell. You can double check with env | grep sgx."
;;
help)
print_helper_text
;;
remove)
$SGX_VULNERABLE_INSTALL/sgxsdk/uninstall
/opt/intel/sgxsdk/uninstall
if [ "$(ls -A /opt/intel/sgxpsw/)" ]; then
/opt/intel/sgxpsw/uninstall
fi
;;
*)
echo "Invalid Input. Only accepting one of $ALLOWED_INPUTS"
;;
esac
