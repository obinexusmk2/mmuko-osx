# CMake generated Testfile for 
# Source directory: /mnt/c/Users/OBINexus/Workspace/rift-pipeline
# Build directory: /mnt/c/Users/OBINexus/Workspace/rift-pipeline
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(pipeline_basic "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/rift_tests" "basic")
set_tests_properties(pipeline_basic PROPERTIES  _BACKTRACE_TRIPLES "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;129;add_test;/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;0;")
add_test(tokenizer_test "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/rift_tests" "tokenizer")
set_tests_properties(tokenizer_test PROPERTIES  _BACKTRACE_TRIPLES "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;130;add_test;/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;0;")
add_test(nsigii_codec_test "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/rift_tests" "nsigii")
set_tests_properties(nsigii_codec_test PROPERTIES  _BACKTRACE_TRIPLES "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;131;add_test;/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;0;")
add_test(riftbridge_test "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/rift_tests" "riftbridge")
set_tests_properties(riftbridge_test PROPERTIES  _BACKTRACE_TRIPLES "/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;132;add_test;/mnt/c/Users/OBINexus/Workspace/rift-pipeline/CMakeLists.txt;0;")
