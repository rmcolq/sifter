# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: CC0-1.0

# CPM Package Lock
# This file should be committed to version control

# seqan3
set (CHOPPER_SEQAN3_VERSION 6b681fb2eae5ab2997d293e99fc6a7f869a20316)
CPMDeclarePackage (seqan3
                   NAME seqan3
                   GIT_TAG ${CHOPPER_SEQAN3_VERSION}
                   GITHUB_REPOSITORY seqan/seqan3
                   SYSTEM TRUE
                   EXCLUDE_FROM_ALL TRUE
                   OPTIONS "INSTALL_SEQAN3 OFF" "CMAKE_MESSAGE_LOG_LEVEL WARNING"
)
# googletest
set (CHOPPER_GOOGLETEST_VERSION 1.15.2)
CPMDeclarePackage (googletest
                   NAME googletest
                   VERSION ${CHOPPER_GOOGLETEST_VERSION}
                   GITHUB_REPOSITORY google/googletest
                   SYSTEM TRUE
                   EXCLUDE_FROM_ALL TRUE
                   OPTIONS "BUILD_GMOCK OFF" "INSTALL_GTEST OFF" "CMAKE_MESSAGE_LOG_LEVEL WARNING"
                           "CMAKE_CXX_STANDARD 20"
)
