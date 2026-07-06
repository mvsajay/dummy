#!/usr/bin/env bash
# =============================================================================
# build_all.sh — Build all simple_benchmarks as RISC-V static ELF binaries
#
# Usage (from anywhere):
#   bash simple_benchmarks/build_all.sh
#   -- or --
#   cd simple_benchmarks/ && bash build_all.sh
#
# All ELF outputs land in:
#   simple_benchmarks/build/
#
# Compiler flags:
#   -mabi=lp64d  : LP64D ABI (64-bit int/long/ptr, double-precision FP regs)
#   -fPIE        : Position-independent executable (required by ELF loader)
#   -static      : Fully static — no dynamic linker needed on target
#   -O1          : Light optimisation — keeps instruction patterns representative
#   -B / -L      : Point to the RISC-V glibc sysroot
# =============================================================================

set -e   # abort on first error

# ---------------------------------------------------------------------------
# Toolchain config
# ---------------------------------------------------------------------------
CC=/opt/riscv64-rv64g/bin/gcc
CFLAGS="-mabi=lp64d -fPIE -static -O1"
GLIBC_DIR=/opt/glibc-rv64g/lib
SYSROOT_FLAGS="-B${GLIBC_DIR} -L${GLIBC_DIR}"

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${SCRIPT_DIR}"
BUILD_DIR="${SCRIPT_DIR}/build"

# Create build directory (no error if it already exists)
mkdir -p "${BUILD_DIR}"

echo "============================================================"
echo " simple_benchmarks — RISC-V ELF build"
echo " Compiler  : ${CC}"
echo " CFLAGS    : ${CFLAGS}"
echo " Sysroot   : ${GLIBC_DIR}"
echo " Source    : ${SRC_DIR}"
echo " Output    : ${BUILD_DIR}"
echo "============================================================"
echo ""

# ---------------------------------------------------------------------------
# Helper: compile one source -> build/<name>.elf
# Usage: build_one <source.c> <output_stem>
# ---------------------------------------------------------------------------
PASS=0
FAIL=0

build_one() {
    local src="${SRC_DIR}/$1"
    local out="${BUILD_DIR}/$2"
    printf "  %-30s -> %s ... " "$1" "$(basename "${out}")"
    if ${CC} ${CFLAGS} ${SYSROOT_FLAGS} "${src}" -o "${out}" 2>/tmp/bench_build_err; then
        local sz
        sz=$(du -h "${out}" | cut -f1)
        echo "OK  [${sz}]"
        PASS=$((PASS + 1))
    else
        echo "FAILED"
        cat /tmp/bench_build_err
        FAIL=$((FAIL + 1))
        # Don't abort — continue building remaining benchmarks
    fi
}

# ---------------------------------------------------------------------------
# Individual benchmarks (each gets its own ELF)
# ---------------------------------------------------------------------------
build_one 01_matrix.c         01_matrix.elf
build_one 02_large_matrix.c   02_large_matrix.elf
build_one 03_pointer_chase.c  03_pointer_chase.elf
build_one 04_merge_sort.c     04_merge_sort.elf
build_one 05_hash_table.c     05_hash_table.elf
build_one 06_fibonacci.c      06_fibonacci.elf
build_one 07_string_manual.c  07_string_manual.elf
build_one 08_graph.c          08_graph.elf
build_one 09_libstring.c      09_libstring.elf

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo ""
echo "============================================================"
echo " Build summary: ${PASS} passed, ${FAIL} failed"
echo "============================================================"
echo ""
echo " ELF files in ${BUILD_DIR}:"
ls -lh "${BUILD_DIR}"/*.elf 2>/dev/null | awk '{printf "   %-35s %s\n", $NF, $5}' \
    || echo "   (none)"
echo ""

# Exit non-zero if any build failed
[ "${FAIL}" -eq 0 ]

# Made with Bob
