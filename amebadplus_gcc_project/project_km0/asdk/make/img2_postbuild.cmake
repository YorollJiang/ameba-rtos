#Note: Previously defined variables cannot be used directly in this file
#unless passed through -D
include(${CMAKE_FILES_DIR}/axf2bin.cmake)

if(CONFIG_MP_SHRINK)
    execute_process(
        COMMAND ${CMAKE_OBJCOPY} -j .ram_image2.entry -j .sram_image2.text.data -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/sram_2.bin
    )
else()
    execute_process(
        COMMAND ${CMAKE_OBJCOPY} -j .ram_image2.entry -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/entry.bin
        COMMAND ${CMAKE_OBJCOPY} -j .sram_image2.text.data -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/sram_2.bin
    )
endif()

file(READ ${IMAGE_TARGET_FOLDER}/target_img2.map content)
string(REPLACE "\n" ";" lines ${content})
foreach(_line ${lines})
    string(REGEX MATCH "^.*__exidx_end" result ${_line})
    if(result)
        break()
    endif()
endforeach()
string(REPLACE " " ";" match_line "${result}")
list(GET match_line 0 ARMExAddr)

if(0x${ARMExAddr} GREATER 0x60000000)
    execute_process(
        COMMAND ${CMAKE_OBJCOPY} -j .psram_image2.text.data -j .ARM.extab -j .ARM.exidx -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/psram_2.bin
        COMMAND ${CMAKE_OBJCOPY} -j .xip_image2.text -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/xip_image2.bin
    )
else()
    execute_process(
        COMMAND ${CMAKE_OBJCOPY} -j .psram_image2.text.data -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/psram_2.bin
        COMMAND ${CMAKE_OBJCOPY} -j .xip_image2.text -j .ARM.extab -j .ARM.exidx -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/xip_image2.bin
    )
endif()

execute_process(
    COMMAND ${CMAKE_OBJCOPY} -j .ram_retention.text -j .ram_retention.entry -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/ram_retention.bin
    COMMAND ${CMAKE_OBJCOPY} -j .coex_trace.text -Obinary ${IMAGE_TARGET_FOLDER}/target_pure_img2.axf ${IMAGE_TARGET_FOLDER}/COEX.trace
)


message( "========== Image manipulating start ==========")

execute_process(
    COMMAND ${PADTOOL} ${IMAGE_TARGET_FOLDER}/xip_image2.bin 32
)
execute_process(
	COMMAND ${PADTOOL} ${IMAGE_TARGET_FOLDER}/entry.bin 32
)
execute_process(
	COMMAND ${PADTOOL} ${IMAGE_TARGET_FOLDER}/sram_2.bin 32
)
execute_process(
	COMMAND ${PADTOOL} ${IMAGE_TARGET_FOLDER}/psram_2.bin 32
)

if(CONFIG_MP_SHRINK)
    execute_process(
        COMMAND ${PREPENDTOOL} ${IMAGE_TARGET_FOLDER}/sram_2.bin  __img2_entry_start__  ${IMAGE_TARGET_FOLDER}/target_img2.map
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E cat ${IMAGE_TARGET_FOLDER}/sram_2_prepend.bin
        OUTPUT_FILE ${IMAGE_TARGET_FOLDER}/km0_image2_all_shrink.bin
    )
    execute_process(
        COMMAND ${IMAGETOOL} ${IMAGE_TARGET_FOLDER}/km0_image2_all_shrink.bin ${BUILD_TYPE}
        WORKING_DIRECTORY ${PROJECTDIR}/..
    )
else()
    execute_process(
        COMMAND ${PREPENDTOOL} ${IMAGE_TARGET_FOLDER}/entry.bin  __img2_entry_start__  ${IMAGE_TARGET_FOLDER}/target_img2.map
    )
    execute_process(
        COMMAND ${PREPENDTOOL} ${IMAGE_TARGET_FOLDER}/sram_2.bin  __sram_image2_start__  ${IMAGE_TARGET_FOLDER}/target_img2.map
    )
    execute_process(
        COMMAND ${PREPENDTOOL} ${IMAGE_TARGET_FOLDER}/psram_2.bin  __psram_image2_start__  ${IMAGE_TARGET_FOLDER}/target_img2.map
    )
    execute_process(
        COMMAND ${PREPENDTOOL} ${IMAGE_TARGET_FOLDER}/xip_image2.bin  __flash_text_start__  ${IMAGE_TARGET_FOLDER}/target_img2.map
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E cat ${IMAGE_TARGET_FOLDER}/xip_image2_prepend.bin ${IMAGE_TARGET_FOLDER}/sram_2_prepend.bin ${IMAGE_TARGET_FOLDER}/psram_2_prepend.bin ${IMAGE_TARGET_FOLDER}/entry_prepend.bin
        OUTPUT_FILE ${IMAGE_TARGET_FOLDER}/km0_image2_all.bin
    )
    execute_process(
        COMMAND ${PADTOOL} ${IMAGE_TARGET_FOLDER}/km0_image2_all.bin 4096
    )
    execute_process(
        COMMAND ${IMAGETOOL} ${IMAGE_TARGET_FOLDER}/km0_image2_all.bin ${BUILD_TYPE}
        WORKING_DIRECTORY ${PROJECTDIR}/..
    )
endif()

if(CONFIG_FATFS_WITHIN_APP_IMG)
    if(EXISTS ${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin AND EXISTS ${BASEDIR}/amebadplus_gcc_project/fatfs.bin)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E rename ${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin ${KM4_BUILDDIR}/asdk/image/km0_km4_app_tmp.bin
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E rename ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns.bin ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns_tmp.bin
        )
        execute_process(
            COMMAND ${PADTOOL} ${KM4_BUILDDIR}/asdk/image/km0_km4_app_tmp.bin 4096
        )
        execute_process(
            COMMAND ${PADTOOL} ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns_tmp.bin 4096
        )
        execute_process(
            COMMAND ${PREPENDTOOL} ${BASEDIR}/amebadplus_gcc_project/fatfs.bin  VFS1_FLASH_BASE_ADDR  ${KM4_BUILDDIR}/asdk/image/target_img2.map
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E cat ${KM4_BUILDDIR}/asdk/image/km0_km4_app_tmp.bin ${BASEDIR}/amebadplus_gcc_project/fatfs_prepend.bin
            OUTPUT_FILE${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E cat ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns_tmp.bin ${BASEDIR}/amebadplus_gcc_project/fatfs_prepend.bin
            OUTPUT_FILE ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns.bin
        )
    endif()
endif()

if(CONFIG_COMPRESS_OTA_IMG)
    if(EXISTS ${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin)
        execute_process(
            COMMAND ${COMPRESSTOOL} ${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin
        )
        execute_process(
            COMMAND ${COMPRESSTOOL} ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns.bin
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy ${KM4_BUILDDIR}/asdk/image/km0_km4_app_compress.bin ${KM4_BUILDDIR}/asdk/image/tmp_app.bin
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns_compress.bin ${KM4_BUILDDIR}/asdk/image/tmp_app_ns.bin
        )
    endif()
else()
    if(EXISTS ${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy ${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin ${KM4_BUILDDIR}/asdk/image/tmp_app.bin
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy ${KM4_BUILDDIR}/asdk/image/km0_km4_app_ns.bin ${KM4_BUILDDIR}/asdk/image/tmp_app_ns.bin
        )
    endif()
endif()

if(CONFIG_UPGRADE_BOOTLOADER)
    if(EXISTS ${KM4_BUILDDIR}/asdk/image/tmp_app.bin)
        execute_process(
            COMMAND ${OTAPREPENDTOOL} ${KM4_BUILDDIR}/asdk/image/tmp_app.bin ${KM4_BUILDDIR}/asdk/image/km4_boot_all.bin
        )
        execute_process(
            COMMAND ${OTAPREPENDTOOL} ${KM4_BUILDDIR}/asdk/image/tmp_app_ns.bin${KM4_BUILDDIR}/asdk/image/km4_boot_all_ns.bin
        )
    endif()
else()
    if(EXISTS ${KM4_BUILDDIR}/asdk/image/tmp_app.bin)
        execute_process(
            COMMAND ${OTAPREPENDTOOL} ${KM4_BUILDDIR}/asdk/image/tmp_app.bin
        )
        execute_process(
            COMMAND ${OTAPREPENDTOOL} ${KM4_BUILDDIR}/asdk/image/tmp_app_ns.bin
        )
    endif()
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove ${KM4_BUILDDIR}/asdk/image/tmp_app.bin
)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove ${KM4_BUILDDIR}/asdk/image/tmp_app_ns.bin
)
message("========== Image manipulating end ==========")
if (CONFIG_MP_INCLUDED)
    set(APP_ALL ${KM4_BUILDDIR}/asdk/image_mp/km0_km4_app_mp.bin)
else()
    set(APP_ALL	${KM4_BUILDDIR}/asdk/image/km0_km4_app.bin)
endif()
set(OTA_ALL	${KM4_BUILDDIR}/asdk/image/ota_all.bin)

if(EXISTS ${APP_ALL})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy ${APP_ALL} ${FINAL_IMAGE_DIR}
    )
endif()

if(EXISTS ${OTA_ALL})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy ${OTA_ALL} ${FINAL_IMAGE_DIR}
    )
endif()

if(NOT CONFIG_AMEBA_RLS)
    message("========== Image analyze start ==========")
    execute_process(
        COMMAND ${CODE_ANALYZE_PYTHON} ${ANALYZE_MP_IMG} ${DAILY_BUILD}
        WORKING_DIRECTORY ${PROJECTDIR}/asdk
    )
    execute_process(
        COMMAND ${STATIC_ANALYZE_PYTHON} ${DAILY_BUILD}
        WORKING_DIRECTORY ${PROJECTDIR}/asdk
    )
    message("========== Image analyze end ==========")
endif()

