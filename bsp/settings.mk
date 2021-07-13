# Copyright 2019 SiFive, Inc #
# SPDX-License-Identifier: Apache-2.0 #
# ----------------------------------- #
# ----------------------------------- #

RISCV_ARCH=rv64imafdc
RISCV_ABI=lp64d
#RISCV_ABI=lp64f
RISCV_CMODEL=medany
RISCV_SERIES=sifive-7-series

TARGET_TAGS=board openocd
TARGET_DHRY_ITERS=20000000
TARGET_CORE_ITERS=5000
