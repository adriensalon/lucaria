if(LUCARIA_BUILD_COMPILERS)

    set(capstone_core_sources
        "upstream/capstone/cs.c"
        "upstream/capstone/Mapping.c"
        "upstream/capstone/MCInst.c"
        "upstream/capstone/MCInstPrinter.c"
        "upstream/capstone/MCInstrDesc.c"
        "upstream/capstone/MCRegisterInfo.c"
        "upstream/capstone/SStream.c"
        "upstream/capstone/utils.c")
    set(capstone_x86_sources
        "upstream/capstone/arch/X86/X86ATTInstPrinter.c"
        "upstream/capstone/arch/X86/X86Disassembler.c"
        "upstream/capstone/arch/X86/X86DisassemblerDecoder.c"
        "upstream/capstone/arch/X86/X86InstPrinterCommon.c"
        "upstream/capstone/arch/X86/X86IntelInstPrinter.c"
        "upstream/capstone/arch/X86/X86Mapping.c"
        "upstream/capstone/arch/X86/X86Module.c")
    set(capstone_arm_sources
        "upstream/capstone/arch/ARM/ARMBaseInfo.c"
        "upstream/capstone/arch/ARM/ARMDisassembler.c"
        "upstream/capstone/arch/ARM/ARMDisassemblerExtension.c"
        "upstream/capstone/arch/ARM/ARMInstPrinter.c"
        "upstream/capstone/arch/ARM/ARMMapping.c"
        "upstream/capstone/arch/ARM/ARMModule.c")
    set(capstone_aarch64_sources
        "upstream/capstone/arch/AArch64/AArch64BaseInfo.c"
        "upstream/capstone/arch/AArch64/AArch64Disassembler.c"
        "upstream/capstone/arch/AArch64/AArch64DisassemblerExtension.c"
        "upstream/capstone/arch/AArch64/AArch64InstPrinter.c"
        "upstream/capstone/arch/AArch64/AArch64Mapping.c"
        "upstream/capstone/arch/AArch64/AArch64Module.c")
    set(capstone_riscv_sources
        "upstream/capstone/arch/RISCV/RISCVBaseInfo.c"
        "upstream/capstone/arch/RISCV/RISCVDisassembler.c"
        "upstream/capstone/arch/RISCV/RISCVDisassemblerExtension.c"
        "upstream/capstone/arch/RISCV/RISCVInstPrinter.c"
        "upstream/capstone/arch/RISCV/RISCVMapping.c"
        "upstream/capstone/arch/RISCV/RISCVModule.c")
    set(capstone_wasm_sources
        "upstream/capstone/arch/WASM/WASMDisassembler.c"
        "upstream/capstone/arch/WASM/WASMInstPrinter.c"
        "upstream/capstone/arch/WASM/WASMMapping.c"
        "upstream/capstone/arch/WASM/WASMModule.c")
    set(lucaria_external_capstone_sources
        ${capstone_core_sources}
        ${capstone_x86_sources}
        ${capstone_arm_sources}
        ${capstone_aarch64_sources}
        ${capstone_riscv_sources}
        ${capstone_wasm_sources})

    add_library(lucaria_external_capstone STATIC EXCLUDE_FROM_ALL ${lucaria_external_capstone_sources})
    set_target_properties(lucaria_external_capstone PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON)

    target_include_directories(lucaria_external_capstone PUBLIC
        "upstream/capstone/include"
        "upstream/capstone/include/capstone")
    target_include_directories(lucaria_external_capstone PRIVATE
        "upstream/capstone"
        "upstream/capstone/arch/ARM"
        "upstream/capstone/arch/AArch64"
        "upstream/capstone/arch/RISCV"
        "upstream/capstone/arch/WASM"
        "upstream/capstone/arch/X86")

    target_compile_definitions(lucaria_external_capstone PUBLIC
        CAPSTONE_HAS_X86
        CAPSTONE_HAS_ARM
        CAPSTONE_HAS_AARCH64
        CAPSTONE_HAS_RISCV
        CAPSTONE_HAS_WASM
        CAPSTONE_USE_SYS_DYN_MEM)

    if(MSVC)
        target_compile_options(lucaria_external_capstone PRIVATE
            /wd4005
            /wd4018
            /wd4098
            /wd4133
            /wd4146
            /wd4244
            /wd4267
            /wd4319
            /wd4334
            /wd4996
            /wd5286
            /wd5287)
    else()
        target_compile_options(lucaria_external_capstone PRIVATE
            -Wno-unused-function
            -Wno-unused-variable
            -Wno-missing-braces)
    endif()

endif()