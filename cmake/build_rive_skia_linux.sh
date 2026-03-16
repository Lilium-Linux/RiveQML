#!/usr/bin/env bash

set -euo pipefail

runtime_dir="$1"
skia_dir="${runtime_dir}/skia/dependencies/skia"
skia_out_dir="${skia_dir}/out/riveqml-linux"

if [[ ! -d "${skia_dir}" ]]; then
    echo "Skia checkout is missing at ${skia_dir}" >&2
    exit 1
fi

(
    cd "${skia_dir}"
    python3 tools/git-sync-deps

    bin/gn gen "${skia_out_dir}" --type=static_library --args=" \
        is_official_build=true \
        extra_cflags=[\"-fPIC\", \"-fno-rtti\", \"-DSK_DISABLE_SKPICTURE\", \"-DSK_DISABLE_TEXT\", \"-DRIVE_OPTIMIZED\", \"-DSK_DISABLE_LEGACY_SHADERCONTEXT\", \"-DSK_DISABLE_LOWP_RASTER_PIPELINE\", \"-DSK_FORCE_RASTER_PIPELINE_BLITTER\", \"-DSK_DISABLE_AAA\", \"-DSK_DISABLE_EFFECT_DESERIALIZATION\"] \
        extra_cflags_cc=[\"-include\", \"cstdint\"] \
        skia_enable_gpu=false \
        skia_use_gl=false \
        skia_use_zlib=true \
        skia_use_system_zlib=false \
        skia_enable_fontmgr_empty=true \
        skia_use_libpng_encode=false \
        skia_use_libpng_decode=true \
        skia_use_system_libpng=false \
        skia_use_system_libjpeg_turbo=false \
        skia_use_libjpeg_turbo_encode=false \
        skia_use_libjpeg_turbo_decode=true \
        skia_use_libwebp_encode=false \
        skia_use_libwebp_decode=true \
        skia_use_system_libwebp=false \
        skia_use_dng_sdk=false \
        skia_use_egl=false \
        skia_use_expat=false \
        skia_use_fontconfig=false \
        skia_use_freetype=false \
        skia_use_system_freetype2=false \
        skia_use_icu=false \
        skia_use_libheif=false \
        skia_use_lua=false \
        skia_use_piex=false \
        skia_use_vulkan=false \
        skia_use_metal=false \
        skia_use_angle=false \
        skia_enable_spirv_validation=false \
        skia_enable_pdf=false \
        skia_enable_skottie=false \
        skia_enable_tools=false \
        skia_enable_skgpu_v1=false \
        skia_gl_standard=\"\" \
    "

    ninja -C "${skia_out_dir}"
)
