#!/bin/bash

echo "Installing dependencies for process monitor..."

# Detect package manager and install ncurses development library
if command -v apt-get &> /dev/null; then
    echo "Detected apt (Debian/Ubuntu)"
    sudo apt-get update
    sudo apt-get install -y libncurses5-dev libncursesw5-dev
elif command -v dnf &> /dev/null; then
    echo "Detected dnf (Fedora)"
    sudo dnf install -y ncurses-devel
elif command -v yum &> /dev/null; then
    echo "Detected yum (RHEL/CentOS)"
    sudo yum install -y ncurses-devel
elif command -v pacman &> /dev/null; then
    echo "Detected pacman (Arch/Manjaro)"
    sudo pacman -S --needed --noconfirm ncurses
elif command -v zypper &> /dev/null; then
    echo "Detected zypper (openSUSE)"
    sudo zypper install -y ncurses-devel
else
    echo "ERROR: Could not detect package manager"
    echo "Please install ncurses development library manually"
    exit 1
fi

echo "Dependencies installed successfully!"

