# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 StarFiveTech, Inc
###################################################
# Build Flags for the Debug Configuration
###################################################

# Set the optimization level
ASFLAGS += -O0
CFLAGS += -O0
CXXFLAGS += -O0

# Enable debug
ASFLAGS += -g
CFLAGS += -g
CXXFLAGS += -g

