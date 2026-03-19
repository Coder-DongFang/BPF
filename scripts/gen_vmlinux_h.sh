#!/usr/bin/env bash
set -e

OUTPUT=${1:-vmlinux.h}

if [ ! -f /sys/kernel/btf/vmlinux ]; then
    echo "ERROR: /sys/kernel/btf/vmlinux not found"
    echo "Make sure your kernel has BTF enabled (CONFIG_DEBUG_INFO_BTF=y)"
    exit 1
fi

if ! command -v bpftool &> /dev/null; then
    echo "ERROR: bpftool not found. Install it first:"
    echo "sudo apt install bpftool"
    exit 1
fi

echo "[+] Generating vmlinux.h from kernel BTF..."
bpftool btf dump file /sys/kernel/btf/vmlinux format c > $OUTPUT

echo "[+] vmlinux.h generated at $OUTPUT"
