if(LUCARIA_BUILD_COMPILERS)

	set(zstd_common_sources
		"upstream/zstd/lib/common/debug.c"
		"upstream/zstd/lib/common/entropy_common.c"
		"upstream/zstd/lib/common/error_private.c"
		"upstream/zstd/lib/common/fse_decompress.c"
		"upstream/zstd/lib/common/pool.c"
		"upstream/zstd/lib/common/threading.c"
		"upstream/zstd/lib/common/xxhash.c"
		"upstream/zstd/lib/common/zstd_common.c")
	set(zstd_decompress_sources
		"upstream/zstd/lib/decompress/huf_decompress.c"
		"upstream/zstd/lib/decompress/zstd_ddict.c"
		"upstream/zstd/lib/decompress/zstd_decompress.c"
		"upstream/zstd/lib/decompress/zstd_decompress_block.c")
	set(zstd_compress_sources
		"upstream/zstd/lib/compress/fse_compress.c"
		"upstream/zstd/lib/compress/hist.c"
		"upstream/zstd/lib/compress/huf_compress.c"
		"upstream/zstd/lib/compress/zstd_compress.c"
		"upstream/zstd/lib/compress/zstd_compress_literals.c"
		"upstream/zstd/lib/compress/zstd_compress_sequences.c"
		"upstream/zstd/lib/compress/zstd_compress_superblock.c"
		"upstream/zstd/lib/compress/zstd_double_fast.c"
		"upstream/zstd/lib/compress/zstd_fast.c"
		"upstream/zstd/lib/compress/zstd_lazy.c"
		"upstream/zstd/lib/compress/zstd_ldm.c"
		"upstream/zstd/lib/compress/zstd_opt.c"
		"upstream/zstd/lib/compress/zstd_preSplit.c"
		"upstream/zstd/lib/compress/zstdmt_compress.c")
	set(zstd_dictbuilder_sources
		"upstream/zstd/lib/dictBuilder/cover.c"
		"upstream/zstd/lib/dictBuilder/divsufsort.c"
		"upstream/zstd/lib/dictBuilder/fastcover.c"
		"upstream/zstd/lib/dictBuilder/zdict.c")
	set(lucaria_external_zstd_sources
		${zstd_common_sources}
		${zstd_decompress_sources}
		${zstd_compress_sources}
		${zstd_dictbuilder_sources})

	add_library(lucaria_external_zstd STATIC EXCLUDE_FROM_ALL ${lucaria_external_zstd_sources})
	set_target_properties(lucaria_external_zstd PROPERTIES
		C_STANDARD 99
		C_STANDARD_REQUIRED ON)

	target_include_directories(lucaria_external_zstd PUBLIC
		"upstream/zstd/lib"
		"upstream/zstd/lib/common"
		"upstream/zstd/lib/decompress"
		"upstream/zstd/lib/compress")

	target_compile_definitions(lucaria_external_zstd PUBLIC ZSTD_STATIC_LINKING_ONLY)
	target_compile_definitions(lucaria_external_zstd PRIVATE XXH_PRIVATE_API)

	if(WIN32)
		target_compile_definitions(lucaria_external_zstd PRIVATE
			ZSTD_DISABLE_ASM)
		target_compile_options(lucaria_external_zstd PRIVATE
			/wd4005
			/wd4244
			/wd4267
			/wd4334
			/wd4996)
	else()
		target_compile_options(lucaria_external_zstd PRIVATE
			-Wno-unused-function
			-Wno-unused-variable
			-Wno-missing-braces)
	endif()

endif()
