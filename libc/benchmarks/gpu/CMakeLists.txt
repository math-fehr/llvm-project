add_subdirectory(timing)

add_custom_target(gpu-benchmark)

function(add_benchmark benchmark_name)
  cmake_parse_arguments(
    "BENCHMARK"
    "" # Optional arguments
    "" # Single value arguments
    "LINK_LIBRARIES" # Multi-value arguments
    ${ARGN}
  )
  if(NOT libc.src.time.clock IN_LIST TARGET_LLVMLIBC_ENTRYPOINTS)
    message(FATAL_ERROR "target does not support clock")
  endif()
  add_libc_hermetic(
    ${benchmark_name}
    IS_BENCHMARK
    LINK_LIBRARIES
      LibcGpuBenchmark.hermetic
      ${BENCHMARK_LINK_LIBRARIES}
    ${BENCHMARK_UNPARSED_ARGUMENTS}
  )
  get_fq_target_name(${benchmark_name} fq_target_name)
  add_dependencies(gpu-benchmark ${fq_target_name})
endfunction(add_benchmark)

add_unittest_framework_library(
  LibcGpuBenchmark
  SRCS
    LibcGpuBenchmark.cpp
    LibcGpuBenchmarkMain.cpp
    BenchmarkLogger.cpp
  HDRS
    LibcGpuBenchmark.h
    BenchmarkLogger.h
  DEPENDS
    libc.src.__support.big_int
    libc.src.__support.c_string
    libc.src.__support.CPP.string
    libc.src.__support.CPP.string_view
    libc.src.__support.CPP.type_traits
    libc.src.__support.CPP.functional
    libc.src.__support.CPP.limits
    libc.src.__support.CPP.algorithm
    libc.src.__support.CPP.atomic
    libc.src.__support.fixed_point.fx_rep
    libc.src.__support.macros.properties.types
    libc.src.__support.OSUtil.osutil
    libc.src.__support.uint128
    libc.src.__support.FPUtil.sqrt
    libc.src.__support.fixedvector
    libc.src.time.clock
    libc.benchmarks.gpu.timing.timing
)

add_subdirectory(src)
