project(FreeImage)

set(freeimage_sources
    ./FreeImage/BitmapAccess.cpp
    ./FreeImage/ColorLookup.cpp
    ./FreeImage/ConversionRGBA16.cpp
    ./FreeImage/ConversionRGBAF.cpp
    ./FreeImage/FreeImage.cpp
    ./FreeImage/FreeImageC.c
    ./FreeImage/FreeImageIO.cpp
    ./FreeImage/GetType.cpp
    ./FreeImage/LFPQuantizer.cpp
    ./FreeImage/MemoryIO.cpp
    ./FreeImage/PixelAccess.cpp
    ./FreeImage/J2KHelper.cpp
    ./FreeImage/MNGHelper.cpp
    ./FreeImage/Plugin.cpp
    ./FreeImage/PluginBMP.cpp
    ./FreeImage/PluginCUT.cpp
    ./FreeImage/PluginDDS.cpp
    ./FreeImage/PluginEXR.cpp
    ./FreeImage/PluginG3.cpp
    ./FreeImage/PluginGIF.cpp
    ./FreeImage/PluginHDR.cpp
    ./FreeImage/PluginICO.cpp
    ./FreeImage/PluginIFF.cpp
    ./FreeImage/PluginJ2K.cpp
    ./FreeImage/PluginJNG.cpp
    ./FreeImage/PluginJP2.cpp
    ./FreeImage/PluginJPEG.cpp
    ./FreeImage/PluginJXR.cpp
    ./FreeImage/PluginKOALA.cpp
    ./FreeImage/PluginMNG.cpp
    ./FreeImage/PluginPCD.cpp
    ./FreeImage/PluginPCX.cpp
    ./FreeImage/PluginPFM.cpp
    ./FreeImage/PluginPICT.cpp
    ./FreeImage/PluginPNG.cpp
    ./FreeImage/PluginPNM.cpp
    ./FreeImage/PluginPSD.cpp
    ./FreeImage/PluginRAS.cpp
    ./FreeImage/PluginRAW.cpp
    ./FreeImage/PluginSGI.cpp
    ./FreeImage/PluginTARGA.cpp
    ./FreeImage/PluginTIFF.cpp
    ./FreeImage/PluginWBMP.cpp
    ./FreeImage/PluginWebP.cpp
    ./FreeImage/PluginXBM.cpp
    ./FreeImage/PluginXPM.cpp
    ./FreeImage/PSDParser.cpp
    ./FreeImage/TIFFLogLuv.cpp
    ./FreeImage/Conversion.cpp
    ./FreeImage/Conversion16_555.cpp
    ./FreeImage/Conversion16_565.cpp
    ./FreeImage/Conversion24.cpp
    ./FreeImage/Conversion32.cpp
    ./FreeImage/Conversion4.cpp
    ./FreeImage/Conversion8.cpp
    ./FreeImage/ConversionFloat.cpp
    ./FreeImage/ConversionRGB16.cpp
    ./FreeImage/ConversionRGBF.cpp
    ./FreeImage/ConversionType.cpp
    ./FreeImage/ConversionUINT16.cpp
    ./FreeImage/Halftoning.cpp
    ./FreeImage/tmoColorConvert.cpp
    ./FreeImage/tmoDrago03.cpp
    ./FreeImage/tmoFattal02.cpp
    ./FreeImage/tmoReinhard05.cpp
    ./FreeImage/ToneMapping.cpp
    ./FreeImage/NNQuantizer.cpp
    ./FreeImage/WuQuantizer.cpp
    ./FreeImage/CacheFile.cpp
    ./FreeImage/MultiPage.cpp
    ./FreeImage/ZLibInterface.cpp
    ./Metadata/Exif.cpp
    ./Metadata/FIRational.cpp
    ./Metadata/FreeImageTag.cpp
    ./Metadata/IPTC.cpp
    ./Metadata/TagConversion.cpp
    ./Metadata/TagLib.cpp
    ./Metadata/XTIFF.cpp
    ./FreeImageToolkit/Background.cpp
    ./FreeImageToolkit/BSplineRotate.cpp
    ./FreeImageToolkit/Channels.cpp
    ./FreeImageToolkit/ClassicRotate.cpp
    ./FreeImageToolkit/Colors.cpp
    ./FreeImageToolkit/CopyPaste.cpp
    ./FreeImageToolkit/Display.cpp
    ./FreeImageToolkit/Flip.cpp
    ./FreeImageToolkit/JPEGTransform.cpp
    ./FreeImageToolkit/MultigridPoissonSolver.cpp
    ./FreeImageToolkit/Rescale.cpp
    ./FreeImageToolkit/Resize.cpp
    #./LibJPEG/jaricom.c
    #./LibJPEG/jcapimin.c
    #./LibJPEG/jcapistd.c 
    #./LibJPEG/jcarith.c 
    #./LibJPEG/jccoefct.c 
    #./LibJPEG/jccolor.c 
    #./LibJPEG/jcdctmgr.c 
    #./LibJPEG/jchuff.c 
    #./LibJPEG/jcinit.c 
    #./LibJPEG/jcmainct.c 
    #./LibJPEG/jcmarker.c 
    #./LibJPEG/jcmaster.c 
    #./LibJPEG/jcomapi.c 
    #./LibJPEG/jcparam.c 
    #./LibJPEG/jcprepct.c 
    #./LibJPEG/jcsample.c 
    #./LibJPEG/jctrans.c 
    #./LibJPEG/jdapimin.c 
    #./LibJPEG/jdapistd.c 
    #./LibJPEG/jdarith.c 
    #./LibJPEG/jdatadst.c 
    #./LibJPEG/jdatasrc.c 
    #./LibJPEG/jdcoefct.c 
    #./LibJPEG/jdcolor.c 
    #./LibJPEG/jddctmgr.c 
    #./LibJPEG/jdhuff.c 
    #./LibJPEG/jdinput.c 
    #./LibJPEG/jdmainct.c 
    #./LibJPEG/jdmarker.c 
    #./LibJPEG/jdmaster.c 
    #./LibJPEG/jdmerge.c 
    #./LibJPEG/jdpostct.c 
    #./LibJPEG/jdsample.c 
    #./LibJPEG/jdtrans.c 
    #./LibJPEG/jerror.c 
    #./LibJPEG/jfdctflt.c 
    #./LibJPEG/jfdctfst.c 
    #./LibJPEG/jfdctint.c 
    #./LibJPEG/jidctflt.c 
    #./LibJPEG/jidctfst.c 
    #./LibJPEG/jidctint.c 
    #./LibJPEG/jmemmgr.c 
    #./LibJPEG/jmemnobs.c 
    #./LibJPEG/jquant1.c 
    #./LibJPEG/jquant2.c 
    #./LibJPEG/jutils.c 
    #./LibJPEG/transupp.c 
#    ./LibPNG/png.c 
#    ./LibPNG/pngerror.c 
#    ./LibPNG/pngget.c 
#    ./LibPNG/pngmem.c 
#    ./LibPNG/pngpread.c 
#    ./LibPNG/pngread.c 
#    ./LibPNG/pngrio.c 
#    ./LibPNG/pngrtran.c 
#    ./LibPNG/pngrutil.c 
#    ./LibPNG/pngset.c 
#    ./LibPNG/pngtrans.c 
#    ./LibPNG/pngwio.c 
#    ./LibPNG/pngwrite.c 
#    ./LibPNG/pngwtran.c 
#    ./LibPNG/pngwutil.c 
#    ./LibTIFF4/tif_aux.c 
#    ./LibTIFF4/tif_close.c 
#    ./LibTIFF4/tif_codec.c 
#    ./LibTIFF4/tif_color.c 
#    ./LibTIFF4/tif_compress.c 
#    ./LibTIFF4/tif_dir.c 
#    ./LibTIFF4/tif_dirinfo.c 
#    ./LibTIFF4/tif_dirread.c 
#    ./LibTIFF4/tif_dirwrite.c 
#    ./LibTIFF4/tif_dumpmode.c 
#    ./LibTIFF4/tif_error.c 
#    ./LibTIFF4/tif_extension.c 
#    ./LibTIFF4/tif_fax3.c 
#    ./LibTIFF4/tif_fax3sm.c 
#    ./LibTIFF4/tif_flush.c 
#    ./LibTIFF4/tif_getimage.c 
#    ./LibTIFF4/tif_jpeg.c 
#    ./LibTIFF4/tif_luv.c 
#    ./LibTIFF4/tif_lzma.c 
#    ./LibTIFF4/tif_lzw.c 
#    ./LibTIFF4/tif_next.c 
#    ./LibTIFF4/tif_ojpeg.c 
#    ./LibTIFF4/tif_open.c 
#    ./LibTIFF4/tif_packbits.c 
#    ./LibTIFF4/tif_pixarlog.c 
#    ./LibTIFF4/tif_predict.c 
#    ./LibTIFF4/tif_print.c 
#    ./LibTIFF4/tif_read.c 
#    ./LibTIFF4/tif_strip.c 
#    ./LibTIFF4/tif_swab.c 
#    ./LibTIFF4/tif_thunder.c 
#    ./LibTIFF4/tif_tile.c 
#    ./LibTIFF4/tif_version.c 
#    ./LibTIFF4/tif_warning.c 
#    ./LibTIFF4/tif_write.c 
#    ./LibTIFF4/tif_zip.c 
#    ./ZLib/adler32.c
#    ./ZLib/compress.c 
#    ./ZLib/crc32.c 
#    ./ZLib/deflate.c 
#    ./ZLib/gzclose.c 
#    ./ZLib/gzlib.c 
#    ./ZLib/gzread.c 
#    ./ZLib/gzwrite.c 
#    ./ZLib/infback.c 
#    ./ZLib/inffast.c 
#    ./ZLib/inflate.c 
#    ./ZLib/inftrees.c 
#    ./ZLib/trees.c 
#    ./ZLib/uncompr.c 
#    ./ZLib/zutil.c 
#    ./LibOpenJPEG/bio.c 
#    ./LibOpenJPEG/cio.c 
#    ./LibOpenJPEG/dwt.c 
#    ./LibOpenJPEG/event.c 
#    ./LibOpenJPEG/function_list.c 
#    ./LibOpenJPEG/image.c 
#    ./LibOpenJPEG/invert.c 
#    ./LibOpenJPEG/j2k.c 
#    ./LibOpenJPEG/jp2.c 
#    ./LibOpenJPEG/mct.c 
#    ./LibOpenJPEG/mqc.c 
#    ./LibOpenJPEG/openjpeg.c 
#    ./LibOpenJPEG/opj_clock.c 
#    ./LibOpenJPEG/pi.c 
#    ./LibOpenJPEG/raw.c 
#    ./LibOpenJPEG/t1.c 
#    ./LibOpenJPEG/t2.c 
#    ./LibOpenJPEG/tcd.c 
#    ./LibOpenJPEG/tgt.c 
    #./OpenEXR/IexMath/IexMathFpu.cpp 
    #./OpenEXR/IlmImf/b44ExpLogTable.cpp 
    #./OpenEXR/IlmImf/ImfAcesFile.cpp 
    #./OpenEXR/IlmImf/ImfAttribute.cpp 
    #./OpenEXR/IlmImf/ImfB44Compressor.cpp 
    #./OpenEXR/IlmImf/ImfBoxAttribute.cpp 
    #./OpenEXR/IlmImf/ImfChannelList.cpp 
    #./OpenEXR/IlmImf/ImfChannelListAttribute.cpp 
    #./OpenEXR/IlmImf/ImfChromaticities.cpp 
    #./OpenEXR/IlmImf/ImfChromaticitiesAttribute.cpp 
    #./OpenEXR/IlmImf/ImfCompositeDeepScanLine.cpp 
    #./OpenEXR/IlmImf/ImfCompressionAttribute.cpp 
    #./OpenEXR/IlmImf/ImfCompressor.cpp 
    #./OpenEXR/IlmImf/ImfConvert.cpp 
    #./OpenEXR/IlmImf/ImfCRgbaFile.cpp 
    #./OpenEXR/IlmImf/ImfDeepCompositing.cpp 
    #./OpenEXR/IlmImf/ImfDeepFrameBuffer.cpp 
    #./OpenEXR/IlmImf/ImfDeepImageStateAttribute.cpp 
    #./OpenEXR/IlmImf/ImfDeepScanLineInputFile.cpp 
    #./OpenEXR/IlmImf/ImfDeepScanLineInputPart.cpp 
    #./OpenEXR/IlmImf/ImfDeepScanLineOutputFile.cpp 
    #./OpenEXR/IlmImf/ImfDeepScanLineOutputPart.cpp 
    #./OpenEXR/IlmImf/ImfDeepTiledInputFile.cpp 
    #./OpenEXR/IlmImf/ImfDeepTiledInputPart.cpp 
    #./OpenEXR/IlmImf/ImfDeepTiledOutputFile.cpp 
    #./OpenEXR/IlmImf/ImfDeepTiledOutputPart.cpp 
    #./OpenEXR/IlmImf/ImfDoubleAttribute.cpp 
    #./OpenEXR/IlmImf/ImfDwaCompressor.cpp 
    #./OpenEXR/IlmImf/ImfEnvmap.cpp 
    #./OpenEXR/IlmImf/ImfEnvmapAttribute.cpp 
    #./OpenEXR/IlmImf/ImfFastHuf.cpp 
    #./OpenEXR/IlmImf/ImfFloatAttribute.cpp 
    #./OpenEXR/IlmImf/ImfFloatVectorAttribute.cpp 
    #./OpenEXR/IlmImf/ImfFrameBuffer.cpp 
    #./OpenEXR/IlmImf/ImfFramesPerSecond.cpp 
    #./OpenEXR/IlmImf/ImfGenericInputFile.cpp 
    #./OpenEXR/IlmImf/ImfGenericOutputFile.cpp 
    #./OpenEXR/IlmImf/ImfHeader.cpp 
    #./OpenEXR/IlmImf/ImfHuf.cpp 
    #./OpenEXR/IlmImf/ImfInputFile.cpp 
    #./OpenEXR/IlmImf/ImfInputPart.cpp 
    #./OpenEXR/IlmImf/ImfInputPartData.cpp 
    #./OpenEXR/IlmImf/ImfIntAttribute.cpp 
    #./OpenEXR/IlmImf/ImfIO.cpp 
    #./OpenEXR/IlmImf/ImfKeyCode.cpp 
    #./OpenEXR/IlmImf/ImfKeyCodeAttribute.cpp 
    #./OpenEXR/IlmImf/ImfLineOrderAttribute.cpp 
    #./OpenEXR/IlmImf/ImfLut.cpp 
    #./OpenEXR/IlmImf/ImfMatrixAttribute.cpp 
    #./OpenEXR/IlmImf/ImfMisc.cpp 
    #./OpenEXR/IlmImf/ImfMultiPartInputFile.cpp 
    #./OpenEXR/IlmImf/ImfMultiPartOutputFile.cpp 
    #./OpenEXR/IlmImf/ImfMultiView.cpp 
    #./OpenEXR/IlmImf/ImfOpaqueAttribute.cpp 
    #./OpenEXR/IlmImf/ImfOutputFile.cpp 
    #./OpenEXR/IlmImf/ImfOutputPart.cpp 
    #./OpenEXR/IlmImf/ImfOutputPartData.cpp 
    #./OpenEXR/IlmImf/ImfPartType.cpp 
    #./OpenEXR/IlmImf/ImfPizCompressor.cpp 
    #./OpenEXR/IlmImf/ImfPreviewImage.cpp 
    #./OpenEXR/IlmImf/ImfPreviewImageAttribute.cpp 
    #./OpenEXR/IlmImf/ImfPxr24Compressor.cpp 
    #./OpenEXR/IlmImf/ImfRational.cpp 
    #./OpenEXR/IlmImf/ImfRationalAttribute.cpp 
    #./OpenEXR/IlmImf/ImfRgbaFile.cpp 
    #./OpenEXR/IlmImf/ImfRgbaYca.cpp 
    #./OpenEXR/IlmImf/ImfRle.cpp 
    #./OpenEXR/IlmImf/ImfRleCompressor.cpp 
    #./OpenEXR/IlmImf/ImfScanLineInputFile.cpp 
    #./OpenEXR/IlmImf/ImfStandardAttributes.cpp 
    #./OpenEXR/IlmImf/ImfStdIO.cpp 
    #./OpenEXR/IlmImf/ImfStringAttribute.cpp 
    #./OpenEXR/IlmImf/ImfStringVectorAttribute.cpp 
    #./OpenEXR/IlmImf/ImfSystemSpecific.cpp 
    #./OpenEXR/IlmImf/ImfTestFile.cpp 
    #./OpenEXR/IlmImf/ImfThreading.cpp 
    #./OpenEXR/IlmImf/ImfTileDescriptionAttribute.cpp 
    #./OpenEXR/IlmImf/ImfTiledInputFile.cpp 
    #./OpenEXR/IlmImf/ImfTiledInputPart.cpp 
    #./OpenEXR/IlmImf/ImfTiledMisc.cpp 
    #./OpenEXR/IlmImf/ImfTiledOutputFile.cpp 
    #./OpenEXR/IlmImf/ImfTiledOutputPart.cpp 
    #./OpenEXR/IlmImf/ImfTiledRgbaFile.cpp 
    #./OpenEXR/IlmImf/ImfTileOffsets.cpp 
    #./OpenEXR/IlmImf/ImfTimeCode.cpp 
    #./OpenEXR/IlmImf/ImfTimeCodeAttribute.cpp 
    #./OpenEXR/IlmImf/ImfVecAttribute.cpp 
    #./OpenEXR/IlmImf/ImfVersion.cpp 
    #./OpenEXR/IlmImf/ImfWav.cpp 
    #./OpenEXR/IlmImf/ImfZip.cpp 
    #./OpenEXR/IlmImf/ImfZipCompressor.cpp 
    #./OpenEXR/Imath/ImathBox.cpp 
    #./OpenEXR/Imath/ImathColorAlgo.cpp 
    #./OpenEXR/Imath/ImathFun.cpp 
    #./OpenEXR/Imath/ImathMatrixAlgo.cpp 
    #./OpenEXR/Imath/ImathRandom.cpp 
    #./OpenEXR/Imath/ImathShear.cpp 
    #./OpenEXR/Imath/ImathVec.cpp 
    #./OpenEXR/Iex/IexBaseExc.cpp 
    #./OpenEXR/Iex/IexThrowErrnoExc.cpp 
    #./OpenEXR/Half/half.cpp 
    #./OpenEXR/IlmThread/IlmThread.cpp 
    #./OpenEXR/IlmThread/IlmThreadMutex.cpp 
    #./OpenEXR/IlmThread/IlmThreadPool.cpp 
    #./OpenEXR/IlmThread/IlmThreadSemaphore.cpp 
    #./OpenEXR/IexMath/IexMathFloatExc.cpp 
    ./LibRawLite/internal/dcraw_common.cpp 
    ./LibRawLite/internal/dcraw_fileio.cpp 
    ./LibRawLite/internal/demosaic_packs.cpp 
    ./LibRawLite/src/libraw_c_api.cpp 
    ./LibRawLite/src/libraw_cxx.cpp 
    ./LibRawLite/src/libraw_datastream.cpp 
    ./LibWebP/src/dec/alpha_dec.c 
    ./LibWebP/src/dec/buffer_dec.c 
    ./LibWebP/src/dec/frame_dec.c 
    ./LibWebP/src/dec/idec_dec.c 
    ./LibWebP/src/dec/io_dec.c 
    ./LibWebP/src/dec/quant_dec.c 
    ./LibWebP/src/dec/tree_dec.c 
    ./LibWebP/src/dec/vp8l_dec.c 
    ./LibWebP/src/dec/vp8_dec.c 
    ./LibWebP/src/dec/webp_dec.c 
    ./LibWebP/src/demux/anim_decode.c 
    ./LibWebP/src/demux/demux.c 
    ./LibWebP/src/dsp/alpha_processing.c 
    ./LibWebP/src/dsp/alpha_processing_mips_dsp_r2.c 
    ./LibWebP/src/dsp/alpha_processing_neon.c 
    ./LibWebP/src/dsp/alpha_processing_sse2.c 
    ./LibWebP/src/dsp/alpha_processing_sse41.c 
    ./LibWebP/src/dsp/cost.c
    ./LibWebP/src/dsp/cost_mips32.c 
    ./LibWebP/src/dsp/cost_mips_dsp_r2.c 
    ./LibWebP/src/dsp/cost_sse2.c 
    ./LibWebP/src/dsp/cpu.c 
    ./LibWebP/src/dsp/dec.c 
    ./LibWebP/src/dsp/dec_clip_tables.c 
    ./LibWebP/src/dsp/dec_mips32.c 
    ./LibWebP/src/dsp/dec_mips_dsp_r2.c 
    ./LibWebP/src/dsp/dec_msa.c 
    ./LibWebP/src/dsp/dec_neon.c 
    ./LibWebP/src/dsp/dec_sse2.c 
    ./LibWebP/src/dsp/dec_sse41.c 
    ./LibWebP/src/dsp/enc.c 
    ./LibWebP/src/dsp/enc_avx2.c 
    ./LibWebP/src/dsp/enc_mips32.c 
    ./LibWebP/src/dsp/enc_mips_dsp_r2.c 
    ./LibWebP/src/dsp/enc_msa.c 
    ./LibWebP/src/dsp/enc_neon.c 
    ./LibWebP/src/dsp/enc_sse2.c 
    ./LibWebP/src/dsp/enc_sse41.c 
    ./LibWebP/src/dsp/filters.c 
    ./LibWebP/src/dsp/filters_mips_dsp_r2.c 
    ./LibWebP/src/dsp/filters_msa.c 
    ./LibWebP/src/dsp/filters_neon.c 
    ./LibWebP/src/dsp/filters_sse2.c 
    ./LibWebP/src/dsp/lossless.c 
    ./LibWebP/src/dsp/lossless_enc.c 
    ./LibWebP/src/dsp/lossless_enc_mips32.c 
    ./LibWebP/src/dsp/lossless_enc_mips_dsp_r2.c 
    ./LibWebP/src/dsp/lossless_enc_msa.c 
    ./LibWebP/src/dsp/lossless_enc_neon.c 
    ./LibWebP/src/dsp/lossless_enc_sse2.c 
    ./LibWebP/src/dsp/lossless_enc_sse41.c 
    ./LibWebP/src/dsp/lossless_mips_dsp_r2.c 
    ./LibWebP/src/dsp/lossless_msa.c 
    ./LibWebP/src/dsp/lossless_neon.c 
    ./LibWebP/src/dsp/lossless_sse2.c 
    ./LibWebP/src/dsp/rescaler.c 
    ./LibWebP/src/dsp/rescaler_mips32.c 
    ./LibWebP/src/dsp/rescaler_mips_dsp_r2.c 
    ./LibWebP/src/dsp/rescaler_msa.c 
    ./LibWebP/src/dsp/rescaler_neon.c 
    ./LibWebP/src/dsp/rescaler_sse2.c 
    ./LibWebP/src/dsp/ssim.c 
    ./LibWebP/src/dsp/ssim_sse2.c 
    ./LibWebP/src/dsp/upsampling.c 
    ./LibWebP/src/dsp/upsampling_mips_dsp_r2.c 
    ./LibWebP/src/dsp/upsampling_msa.c 
    ./LibWebP/src/dsp/upsampling_neon.c 
    ./LibWebP/src/dsp/upsampling_sse2.c 
    ./LibWebP/src/dsp/upsampling_sse41.c 
    ./LibWebP/src/dsp/yuv.c 
    ./LibWebP/src/dsp/yuv_mips32.c 
    ./LibWebP/src/dsp/yuv_mips_dsp_r2.c 
    ./LibWebP/src/dsp/yuv_neon.c 
    ./LibWebP/src/dsp/yuv_sse2.c 
    ./LibWebP/src/dsp/yuv_sse41.c 
    ./LibWebP/src/enc/alpha_enc.c 
    ./LibWebP/src/enc/analysis_enc.c 
    ./LibWebP/src/enc/backward_references_cost_enc.c 
    ./LibWebP/src/enc/backward_references_enc.c 
    ./LibWebP/src/enc/config_enc.c 
    ./LibWebP/src/enc/cost_enc.c 
    ./LibWebP/src/enc/filter_enc.c 
    ./LibWebP/src/enc/frame_enc.c 
    ./LibWebP/src/enc/histogram_enc.c 
    ./LibWebP/src/enc/iterator_enc.c 
    ./LibWebP/src/enc/near_lossless_enc.c 
    ./LibWebP/src/enc/picture_csp_enc.c 
    ./LibWebP/src/enc/picture_enc.c 
    ./LibWebP/src/enc/picture_psnr_enc.c 
    ./LibWebP/src/enc/picture_rescale_enc.c 
    ./LibWebP/src/enc/picture_tools_enc.c 
    ./LibWebP/src/enc/predictor_enc.c 
    ./LibWebP/src/enc/quant_enc.c 
    ./LibWebP/src/enc/syntax_enc.c 
    ./LibWebP/src/enc/token_enc.c 
    ./LibWebP/src/enc/tree_enc.c 
    ./LibWebP/src/enc/vp8l_enc.c 
    ./LibWebP/src/enc/webp_enc.c 
    ./LibWebP/src/mux/anim_encode.c 
    ./LibWebP/src/mux/muxedit.c 
    ./LibWebP/src/mux/muxinternal.c 
    ./LibWebP/src/mux/muxread.c 
    ./LibWebP/src/utils/bit_reader_utils.c 
    ./LibWebP/src/utils/bit_writer_utils.c 
    ./LibWebP/src/utils/color_cache_utils.c 
    ./LibWebP/src/utils/filters_utils.c 
    ./LibWebP/src/utils/huffman_encode_utils.c 
    ./LibWebP/src/utils/huffman_utils.c 
    ./LibWebP/src/utils/quant_levels_dec_utils.c 
    ./LibWebP/src/utils/quant_levels_utils.c 
    ./LibWebP/src/utils/random_utils.c 
    ./LibWebP/src/utils/rescaler_utils.c 
    ./LibWebP/src/utils/thread_utils.c 
    ./LibWebP/src/utils/utils.c 
    #./LibJXR/image/decode/decode.c 
    #./LibJXR/image/decode/JXRTranscode.c 
    #./LibJXR/image/decode/postprocess.c 
    #./LibJXR/image/decode/segdec.c 
    #./LibJXR/image/decode/strdec.c 
    #./LibJXR/image/decode/strdec_x86.c 
    #./LibJXR/image/decode/strInvTransform.c 
    #./LibJXR/image/decode/strPredQuantDec.c 
    #./LibJXR/image/encode/encode.c 
    #./LibJXR/image/encode/segenc.c 
    #./LibJXR/image/encode/strenc.c 
    #./LibJXR/image/encode/strenc_x86.c 
    #./LibJXR/image/encode/strFwdTransform.c 
    #./LibJXR/image/encode/strPredQuantEnc.c 
    #./LibJXR/image/sys/adapthuff.c 
    #./LibJXR/image/sys/image.c 
    #./LibJXR/image/sys/strcodec.c 
    #./LibJXR/image/sys/strPredQuant.c 
    #./LibJXR/image/sys/strTransform.c 
    #./LibJXR/jxrgluelib/JXRGlue.c 
    #./LibJXR/jxrgluelib/JXRGlueJxr.c 
    #./LibJXR/jxrgluelib/JXRGluePFC.c 
    #./LibJXR/jxrgluelib/JXRMeta.c 
    #./Wrapper/FreeImagePlus/src/fipImage.cpp
    #./Wrapper/FreeImagePlus/src/fipMemoryIO.cpp
    #./Wrapper/FreeImagePlus/src/fipMetadataFind.cpp
    #./Wrapper/FreeImagePlus/src/fipMultiPage.cpp
    #./Wrapper/FreeImagePlus/src/fipTag.cpp
    #./Wrapper/FreeImagePlus/src/fipWinImage.cpp
    #./Wrapper/FreeImagePlus/src/FreeImagePlus.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Metadata
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeImageToolkit
    #${CMAKE_CURRENT_SOURCE_DIR}/LibJPEG
    #${CMAKE_CURRENT_SOURCE_DIR}/LibPNG
    #${CMAKE_CURRENT_SOURCE_DIR}/LibTIFF4
    #${CMAKE_CURRENT_SOURCE_DIR}/ZLib
    #${CMAKE_CURRENT_SOURCE_DIR}/LibOpenJPEG
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR/Half
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR/Iex
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR/IlmImf
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR/IlmThread
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR/Imath
    #${CMAKE_CURRENT_SOURCE_DIR}/OpenEXR/IexMath
    ${CMAKE_CURRENT_SOURCE_DIR}/LibRawLite
    ${CMAKE_CURRENT_SOURCE_DIR}/LibRawLite/dcraw
    ${CMAKE_CURRENT_SOURCE_DIR}/LibRawLite/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/LibRawLite/libraw
    ${CMAKE_CURRENT_SOURCE_DIR}/LibRawLite/src
    ${CMAKE_CURRENT_SOURCE_DIR}/LibWebP
    #${CMAKE_CURRENT_SOURCE_DIR}/LibJXR
    #${CMAKE_CURRENT_SOURCE_DIR}/LibJXR/common/include
    #${CMAKE_CURRENT_SOURCE_DIR}/LibJXR/image/sys
    #${CMAKE_CURRENT_SOURCE_DIR}/LibJXR/jxrgluelib
    #${CMAKE_CURRENT_SOURCE_DIR}/FreeImagePlus
)

add_definitions(
    "-DFREEIMAGE_EXPORTS"
    "-DLIBRAW_NODLL"
    "-DOPJ_STATIC"
#    "-DDISABLE_PERF_MEASUREMENT"
)

if(WIN32)
    add_definitions("-DWINVER=0x0500")
    if(MSVC)
        add_definitions("-D_CRT_SECURE_NO_WARNINGS=1")
    endif()
endif()

add_definitions("-DFREEIMAGE_COLORORDER=1")

add_library(FreeImage SHARED ${freeimage_sources})

target_link_libraries(FreeImage PRIVATE
    ${ZLIB_LIBRARY}
    ${LIBJPEG_LIBRARY}
    ${EXR_LIBRARY}
    ${OPENJPEG_LIBRARY}
    ${PNG_LIBRARY}
    ${TIFF_LIBRARY}
    LibJXR
)

target_include_directories(FreeImage PRIVATE
    ${LIBJPEG_INCLUDE_DIR}
    ${OPENJPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
)


