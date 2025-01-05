// ==========================================================
// Tag library
//
// Design and implementation by
// - Herve Drolon <drolon@infonie.fr>
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

// ==========================================================
// Implementation notes : 
// ----------------------
// The tag info tables declared in this file should probably 
// be loaded from an XML file. 
// This would allow internationalization features and also 
// more extensibility. 
// Maybe in a future release ? 
// ==========================================================

#ifdef _MSC_VER 
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters
#endif

#include "FreeImage.h"
#include "Utilities.h"
#include "FreeImageTag.h"

/**
 HOW-TO : add a new TagInfo table
 --------------------------------------------------------------------------
 1) add a table identifier in the TagLib class definition (see enum MDMODEL)
 2) declare the tag table as static and use a 0/NULL value as last entry
 3) initialize the table in TagLib::TagLib
 4) provide a conversion in TagLib::getFreeImageModel
*/

// --------------------------------------------------------------------------
// EXIF standard tags definition
// --------------------------------------------------------------------------

static TagInfo
  exif_exif_tag_table[] =
  {
    {  0x0100, (char *) "ImageWidth", (char *) "Image width"},
    {  0x0101, (char *) "ImageLength", (char *) "Image height"},
    {  0x0102, (char *) "BitsPerSample", (char *) "Number of bits per component"},
    {  0x0103, (char *) "Compression", (char *) "Compression scheme"},
    {  0x0106, (char *) "PhotometricInterpretation", (char *) "Pixel composition"},
    {  0x010A, (char *) "FillOrder", (char*) nullptr},
    {  0x010D, (char *) "DocumentName", (char *) nullptr},
    {  0x010E, (char *) "ImageDescription", (char *) "Image title"},
    {  0x010F, (char *) "Make", (char *) "Image input equipment manufacturer"},
    {  0x0110, (char *) "Model", (char *) "Image input equipment model"},
    {  0x0111, (char *) "StripOffsets", (char *) "Image data location"},
    {  0x0112, (char *) "Orientation", (char *) "Orientation of image"},
    {  0x0115, (char *) "SamplesPerPixel", (char *) "Number of components"},
    {  0x0116, (char *) "RowsPerStrip", (char *) "Number of rows per strip"},
    {  0x0117, (char *) "StripByteCounts", (char *) "Bytes per compressed strip"},
    {  0x011A, (char *) "XResolution", (char *) "Image resolution in width direction"},
    {  0x011B, (char *) "YResolution", (char *) "Image resolution in height direction"},
    {  0x011C, (char *) "PlanarConfiguration", (char *) "Image data arrangement"},
	{  0x011D, (char *) "PageName", (char *) "Name of the page"},
	{  0x011E, (char *) "XPosition", (char *) "X position of the image"},
	{  0x011F, (char *) "YPosition", (char *) "Y position of the image"},
    {  0x0128, (char *) "ResolutionUnit", (char *) "Unit of X and Y resolution"},
	{  0x0129, (char *) "PageNumber", (char *) "Page number"},
    {  0x012D, (char *) "TransferFunction", (char *) "Transfer function"},
    {  0x0131, (char *) "Software", (char *) "Software used"},
    {  0x0132, (char *) "DateTime", (char *) "File change date and time"},
    {  0x013B, (char *) "Artist", (char *) "Person who created the image"},
	{  0x013C, (char *) "HostComputer", (char *) "Host computer used to generate the image"},
    {  0x013E, (char *) "WhitePoint", (char *) "White point chromaticity"},
    {  0x013F, (char *) "PrimaryChromaticities", (char *) "Chromaticities of primaries"},
    {  0x0156, (char *) "TransferRange", (char *) nullptr},
    {  0x0200, (char *) "JPEGProc", (char *) nullptr},
    {  0x0201, (char *) "JPEGInterchangeFormat", (char *) "Offset to JPEG SOI"},
    {  0x0202, (char *) "JPEGInterchangeFormatLength", (char *) "Bytes of JPEG data"},
    {  0x0211, (char *) "YCbCrCoefficients", (char *) "Color space transformation matrix coefficients"},
    {  0x0212, (char *) "YCbCrSubSampling", (char *) "Subsampling ratio of Y to C"},
    {  0x0213, (char *) "YCbCrPositioning", (char *) "Y and C positioning"},
    {  0x0214, (char *) "ReferenceBlackWhite", (char *) "Pair of black and white reference values"},
    {  0x828D, (char *) "CFARepeatPatternDim", (char *) nullptr},
    {  0x828E, (char *) "CFAPattern", (char *) nullptr},
    {  0x828F, (char *) "BatteryLevel", (char *) nullptr},
    {  0x8298, (char *) "Copyright", (char *) "Copyright holder"},
    {  0x829A, (char *) "ExposureTime", (char *) "Exposure time"},
    {  0x829D, (char *) "FNumber", (char *) "F number"},
    {  0x83BB, (char *) "IPTC/NAA", (char *) nullptr},
    {  0x8773, (char *) "InterColorProfile", (char *) "ICC profile data"},
    {  0x8822, (char *) "ExposureProgram", (char *) "Exposure program"},
    {  0x8824, (char *) "SpectralSensitivity", (char *) "Spectral sensitivity"},
    {  0x8825, (char *) "GPSInfo", (char *) nullptr},
    {  0x8827, (char *) "ISOSpeedRatings", (char *) "ISO speed rating"},
    {  0x8828, (char *) "OECF", (char *) "Optoelectric conversion factor"},
    {  0x9000, (char *) "ExifVersion", (char *) "Exif version"},
    {  0x9003, (char *) "DateTimeOriginal", (char *) "Date and time of original data generation"},
    {  0x9004, (char *) "DateTimeDigitized", (char *) "Date and time of digital data generation"},
    {  0x9101, (char *) "ComponentsConfiguration", (char *) "Meaning of each component"},
    {  0x9102, (char *) "CompressedBitsPerPixel", (char *) "Image compression mode"},
    {  0x9201, (char *) "ShutterSpeedValue", (char *) "Shutter speed"},
    {  0x9202, (char *) "ApertureValue", (char *) "Aperture"},
    {  0x9203, (char *) "BrightnessValue", (char *) "Brightness"},
    {  0x9204, (char *) "ExposureBiasValue", (char *) "Exposure bias"},
    {  0x9205, (char *) "MaxApertureValue", (char *) "Maximum lens aperture"},
    {  0x9206, (char *) "SubjectDistance", (char *) "Subject distance"},
    {  0x9207, (char *) "MeteringMode", (char *) "Metering mode"},
    {  0x9208, (char *) "LightSource", (char *) "Light source"},
    {  0x9209, (char *) "Flash", (char *) "Flash"},
    {  0x920A, (char *) "FocalLength", (char *) "Lens focal length"},
	{  0x9214, (char *) "SubjectArea", (char *) "Subject area"},
    {  0x927C, (char *) "MakerNote", (char *) "Manufacturer notes"},
    {  0x9286, (char *) "UserComment", (char *) "User comments"},
    {  0x9290, (char *) "SubSecTime", (char *) "DateTime subseconds"},
    {  0x9291, (char *) "SubSecTimeOriginal", (char *) "DateTimeOriginal subseconds"},
    {  0x9292, (char *) "SubSecTimeDigitized", (char *) "DateTimeDigitized subseconds"},
    {  0xA000, (char *) "FlashPixVersion", (char *) "Supported Flashpix version"},
    {  0xA001, (char *) "ColorSpace", (char *) "Color space information"},
    {  0xA002, (char *) "PixelXDimension", (char *) "Valid image width"},
    {  0xA003, (char *) "PixelYDimension", (char *) "Valid image height"},
    {  0xA004, (char *) "RelatedSoundFile", (char *) "Related audio file"},
    {  0xA005, (char *) "InteroperabilityOffset", (char *) nullptr},
    {  0xA20B, (char *) "FlashEnergy", (char *) "Flash energy"},
    {  0xA20C, (char *) "SpatialFrequencyResponse", (char *) "Spatial frequency response"},
    {  0xA20E, (char *) "FocalPlaneXResolution", (char *) "Focal plane X resolution"},
    {  0xA20F, (char *) "FocalPlaneYResolution", (char *) "Focal plane Y resolution"},
    {  0xA210, (char *) "FocalPlaneResolutionUnit", (char *) "Focal plane resolution unit"},
    {  0xA214, (char *) "SubjectLocation", (char *) "Subject location"},
    {  0xA215, (char *) "ExposureIndex", (char *) "Exposure index"},
    {  0xA217, (char *) "SensingMethod", (char *) "Sensing method"},
    {  0xA300, (char *) "FileSrc", (char *) "File source"},
    {  0xA301, (char *) "SceneType", (char *) "Scene type"},
    {  0xA302, (char *) "CFAPattern", (char *) "CFA pattern"},
    {  0xA401, (char *) "CustomRendered", (char *) "Custom image processing"},
    {  0xA402, (char *) "ExposureMode", (char *) "Exposure mode"},
    {  0xA403, (char *) "WhiteBalance", (char *) "White balance"},
    {  0xA404, (char *) "DigitalZoomRatio", (char *) "Digital zoom ratio"},
    {  0xA405, (char *) "FocalLengthIn35mmFilm", (char *) "Focal length in 35 mm film"},
    {  0xA406, (char *) "SceneCaptureType", (char *) "Scene capture type"},
    {  0xA407, (char *) "GainControl", (char *) "Gain control"},
    {  0xA408, (char *) "Contrast", (char *) "Contrast"},
    {  0xA409, (char *) "Saturation", (char *) "Saturation"},
    {  0xA40A, (char *) "Sharpness", (char *) "Sharpness"},
    {  0xA40B, (char *) "DeviceSettingDescription", (char *) "Device settings description"},
    {  0xA40C, (char *) "SubjectDistanceRange", (char *) "Subject distance range"},
    {  0xA420, (char *) "ImageUniqueID", (char *) "Unique image ID"},
    {  0xA430, (char *) "CameraOwnerName", (char *) "Camera owner name"},
    {  0xA431, (char *) "BodySerialNumber", (char *) "Body serial number"},
    {  0xA432, (char *) "LensSpecification", (char *) "Lens specification"},
    {  0xA433, (char *) "LensMake", (char *) "Lens make"},
    {  0xA434, (char *) "LensModel", (char *) "Lens model"},
    {  0xA435, (char *) "LensSerialNumber", (char *) "Lens serial number"},

	// These tags are not part of the Exiv v2.3 specifications but are often loaded by applications as Exif data
	{  0x4746, (char *) "Rating", (char *) "Rating tag used by Windows"},
	{  0x4749, (char *) "RatingPercent", (char *) "Rating tag used by Windows, value in percent"},
	{  0x9C9B, (char *) "XPTitle", (char *) "Title tag used by Windows, encoded in UCS2"},
	{  0x9C9C, (char *) "XPComment", (char *) "Comment tag used by Windows, encoded in UCS2"},
	{  0x9C9D, (char *) "XPAuthor", (char *) "Author tag used by Windows, encoded in UCS2"},
	{  0x9C9E, (char *) "XPKeywords", (char *) "Keywords tag used by Windows, encoded in UCS2"},
	{  0x9C9F, (char *) "XPSubject", (char *) "Subject tag used by Windows, encoded in UCS2"},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// EXIF GPS tags definition
// --------------------------------------------------------------------------

static TagInfo
  exif_gps_tag_table[] =
  {
    {  0x0000, (char *) "GPSVersionID", (char *) "GPS tag version"},
	{  0x0001, (char *) "GPSLatitudeRef", (char *) "North or South Latitude"},
    {  0x0002, (char *) "GPSLatitude", (char *) "Latitude"},
    {  0x0003, (char *) "GPSLongitudeRef", (char *) "East or West Longitude"},
    {  0x0004, (char *) "GPSLongitude", (char *) "Longitude"},
    {  0x0005, (char *) "GPSAltitudeRef", (char *) "Altitude reference"},
    {  0x0006, (char *) "GPSAltitude", (char *) "Altitude"},
    {  0x0007, (char *) "GPSTimeStamp", (char *) "GPS time (atomic clock)"},
    {  0x0008, (char *) "GPSSatellites", (char *) "GPS satellites used for measurement"},
    {  0x0009, (char *) "GPSStatus", (char *) "GPS receiver status"},
    {  0x000A, (char *) "GPSMeasureMode", (char *) "GPS measurement mode"},
    {  0x000B, (char *) "GPSDOP", (char *) "Measurement precision"},
    {  0x000C, (char *) "GPSSpeedRef", (char *) "Speed unit"},
    {  0x000D, (char *) "GPSSpeed", (char *) "Speed of GPS receiver"},
    {  0x000E, (char *) "GPSTrackRef", (char *) "Reference for direction of movement"},
    {  0x000F, (char *) "GPSTrack", (char *) "Direction of movement"},
    {  0x0010, (char *) "GPSImgDirectionRef", (char *) "Reference for direction of image"},
    {  0x0011, (char *) "GPSImgDirection", (char *) "Direction of image"},
    {  0x0012, (char *) "GPSMapDatum", (char *) "Geodetic survey data used"},
    {  0x0013, (char *) "GPSDestLatitudeRef", (char *) "Reference for latitude of destination"},
    {  0x0014, (char *) "GPSDestLatitude", (char *) "Latitude of destination"},
    {  0x0015, (char *) "GPSDestLongitudeRef", (char *) "Reference for longitude of destination"},
    {  0x0016, (char *) "GPSDestLongitude", (char *) "Longitude of destination"},
    {  0x0017, (char *) "GPSDestBearingRef", (char *) "Reference for bearing of destination"},
    {  0x0018, (char *) "GPSDestBearing", (char *) "Bearing of destination"},
    {  0x0019, (char *) "GPSDestDistanceRef", (char *) "Reference for distance to destination"},
    {  0x001A, (char *) "GPSDestDistance", (char *) "Distance to destination"},
    {  0x001B, (char *) "GPSProcessingMethod", (char *) "Name of GPS processing method"},
    {  0x001C, (char *) "GPSAreaInformation", (char *) "Name of GPS area"},
    {  0x001D, (char *) "GPSDateStamp", (char *) "GPS date"},
    {  0x001E, (char *) "GPSDifferential", (char *) "GPS differential correction"},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// EXIF interoperability tags definition
// --------------------------------------------------------------------------

static TagInfo
  exif_interop_tag_table[] =
  {
    {  0x0001, (char *) "InteroperabilityIndex", (char *) "Interoperability Identification"},
    {  0x0002, (char *) "InteroperabilityVersion", (char *) "Interoperability version"},
    {  0x1000, (char *) "RelatedImageFileFormat", (char *) "File format of image file"},
    {  0x1001, (char *) "RelatedImageWidth", (char *) "Image width"},
    {  0x1002, (char *) "RelatedImageLength", (char *) "Image height"},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// EXIF maker note tags definition
// --------------------------------------------------------------------------

/**
Canon maker note
*/
static TagInfo
  exif_canon_tag_table[] =
  {
    {  0x0001, (char *) "CanonCameraSettings", (char *) "Canon CameraSettings Tags"},
    {  0x0002, (char *) "CanonFocalLength", (char *) "Canon FocalLength Tags"},
    {  0x0003, (char *) "CanonFlashInfo?", (char *) nullptr},
    {  0x0004, (char *) "CanonShotInfo", (char *) "Canon ShotInfo Tags"},
    {  0x0005, (char *) "CanonPanorama", (char *) "Canon Panorama Tags"},
    {  0x0006, (char *) "CanonImageType", (char *) nullptr},
    {  0x0007, (char *) "CanonFirmwareVersion", (char *) nullptr},
    {  0x0008, (char *) "FileNumber", (char *) nullptr},
    {  0x0009, (char *) "OwnerName", (char *) nullptr},
    {  0x000A, (char *) "UnknownD30", (char *) "Canon UnknownD30 Tags"},
    {  0x000C, (char *) "SerialNumber", (char *) nullptr},
    {  0x000D, (char *) "CanonCameraInfo", (char *) "Canon CameraInfo Tags"},
    {  0x000E, (char *) "CanonFileLength", (char *) nullptr},
    {  0x000F, (char *) "CanonCustomFunctions", (char *) "Custom Functions"},
    {  0x0010, (char *) "CanonModelID", (char *) nullptr},
    {  0x0012, (char *) "CanonAFInfo", (char *) "Canon AFInfo Tags"},
    {  0x0013, (char *) "ThumbnailImageValidArea", (char *) nullptr},
    {  0x0015, (char *) "SerialNumberFormat", (char *) nullptr},
    {  0x001A, (char *) "SuperMacro", (char *) nullptr},
    {  0x001C, (char *) "DateStampMode", (char *) nullptr},
    {  0x001D, (char *) "MyColors", (char *) nullptr},
    {  0x001E, (char *) "FirmwareRevision", (char *) nullptr},
    {  0x0023, (char *) "Categories", (char *) nullptr},
    {  0x0024, (char *) "FaceDetect1", (char *) nullptr},
    {  0x0025, (char *) "FaceDetect2", (char *) nullptr},
    {  0x0026, (char *) "CanonAFInfo2", (char *) "Canon AFInfo2 Tags"},
    {  0x0028, (char *) "ImageUniqueID", (char *) nullptr},
    {  0x0081, (char *) "RawDataOffset", (char *) nullptr},
    {  0x0083, (char *) "OriginalDecisionDataOffset", (char *) nullptr},
    {  0x0090, (char *) "CustomFunctions1D", (char *) "CanonCustom Functions1D Tags"},
    {  0x0091, (char *) "PersonalFunctions", (char *) "CanonCustom PersonalFuncs Tags"},
    {  0x0092, (char *) "PersonalFunctionValues", (char *) "CanonCustom PersonalFuncValues Tags"},
    {  0x0093, (char *) "CanonFileInfo", (char *) "Canon FileInfo Tags"},
    {  0x0094, (char *) "AFPointsInFocus1D", (char *) nullptr},
    {  0x0095, (char *) "LensModel", (char *) nullptr},
    {  0x0096, (char *) "SerialInfo", (char *) nullptr},
    {  0x0097, (char *) "DustRemovalData", (char *) nullptr},
	{  0x0098, (char *) "CropInfo", (char *) nullptr},
    {  0x0099, (char *) "CustomFunctions2", (char *) nullptr},
	{  0x009A, (char *) "AspectInfo", (char *) nullptr},
    {  0x00A0, (char *) "ProcessingInfo", (char *) nullptr},
    {  0x00A1, (char *) "ToneCurveTable", (char *) nullptr},
    {  0x00A2, (char *) "SharpnessTable", (char *) nullptr},
    {  0x00A3, (char *) "SharpnessFreqTable", (char *) nullptr},
    {  0x00A4, (char *) "WhiteBalanceTable", (char *) nullptr},
    {  0x00A9, (char *) "ColorBalance", (char *) nullptr},
    {  0x00AA, (char *) "MeasuredColor", (char *) nullptr},
    {  0x00AE, (char *) "ColorTemperature", (char *) nullptr},
    {  0x00B0, (char *) "CanonFlags", (char *) nullptr},
    {  0x00B1, (char *) "ModifiedInfo", (char *) nullptr},
    {  0x00B2, (char *) "ToneCurveMatching", (char *) nullptr},
    {  0x00B3, (char *) "WhiteBalanceMatching", (char *) nullptr},
    {  0x00B4, (char *) "ColorSpace", (char *) nullptr},
    {  0x00B6, (char *) "PreviewImageInfo", (char *) nullptr},
    {  0x00D0, (char *) "VRDOffset", (char *) "Offset of VRD 'recipe data' if it exists"},
    {  0x00E0, (char *) "SensorInfo", (char *) nullptr},
    {  0x4001, (char *) "ColorData", (char *) "Canon ColorData Tags"},
    {  0x4002, (char *) "CRWParam?", (char *) nullptr},
    {  0x4003, (char *) "ColorInfo", (char *) nullptr},
    {  0x4005, (char *) "Flavor?", (char *) nullptr},
    {  0x4008, (char *) "BlackLevel?", (char *) nullptr},
	{  0x4010, (char *) "CustomPictureStyleFileName", (char *) nullptr},
    {  0x4013, (char *) "AFMicroAdj", (char *) nullptr},
    {  0x4015, (char *) "VignettingCorr", (char *) nullptr},
    {  0x4016, (char *) "VignettingCorr2", (char *) nullptr},
    {  0x4018, (char *) "LightingOpt", (char *) nullptr},
	{  0x4019, (char *) "LensInfo", (char *) nullptr},
	{  0x4020, (char *) "AmbienceInfo", (char *) nullptr},
	{  0x4024, (char *) "FilterInfo", (char *) nullptr},

	// These 'sub'-tag values have been created for consistency -- they don't exist within the exif segment

	// Fields under tag 0x0001 (we add 0xC100 to make unique tag id)
    {  0xC100 + 1, (char *) "CameraSettings:MacroMode", (char *) nullptr},
    {  0xC100 + 2, (char *) "CameraSettings:SelfTimer", (char *) nullptr},
    {  0xC100 + 3, (char *) "CameraSettings:Quality", (char *) nullptr},
    {  0xC100 + 4, (char *) "CameraSettings:CanonFlashMode", (char *) nullptr},
    {  0xC100 + 5, (char *) "CameraSettings:ContinuousDrive", (char *) nullptr},
	{  0xC100 + 6, (char *) "CameraSettings:0x0006", (char *) nullptr},
    {  0xC100 + 7, (char *) "CameraSettings:FocusMode", (char *) nullptr},
	{  0xC100 + 8, (char *) "CameraSettings:0x0008", (char *) nullptr},
    {  0xC100 + 9, (char *) "CameraSettings:RecordMode", (char *) nullptr},
    {  0xC100 + 10, (char *) "CameraSettings:CanonImageSize", (char *) nullptr},
    {  0xC100 + 11, (char *) "CameraSettings:EasyMode", (char *) nullptr},
    {  0xC100 + 12, (char *) "CameraSettings:DigitalZoom", (char *) nullptr},
    {  0xC100 + 13, (char *) "CameraSettings:Contrast", (char *) nullptr},
    {  0xC100 + 14, (char *) "CameraSettings:Saturation", (char *) nullptr},
    {  0xC100 + 15, (char *) "CameraSettings:Sharpness", (char *) nullptr},
    {  0xC100 + 16, (char *) "CameraSettings:CameraISO", (char *) nullptr},
    {  0xC100 + 17, (char *) "CameraSettings:MeteringMode", (char *) nullptr},
    {  0xC100 + 18, (char *) "CameraSettings:FocusRange", (char *) nullptr},
    {  0xC100 + 19, (char *) "CameraSettings:AFPoint", (char *) nullptr},
    {  0xC100 + 20, (char *) "CameraSettings:CanonExposureMode", (char *) nullptr},
	{  0xC100 + 21, (char *) "CameraSettings:0x0015", (char *) nullptr},
    {  0xC100 + 22, (char *) "CameraSettings:LensType", (char *) nullptr},
    {  0xC100 + 23, (char *) "CameraSettings:LongFocal", (char *) nullptr},
    {  0xC100 + 24, (char *) "CameraSettings:ShortFocal", (char *) nullptr},
    {  0xC100 + 25, (char *) "CameraSettings:FocalUnits", (char *) "Focal Units per mm"},
    {  0xC100 + 26, (char *) "CameraSettings:MaxAperture", (char *) nullptr},
    {  0xC100 + 27, (char *) "CameraSettings:MinAperture", (char *) nullptr},
    {  0xC100 + 28, (char *) "CameraSettings:FlashActivity", (char *) nullptr},
    {  0xC100 + 29, (char *) "CameraSettings:FlashBits", (char *) nullptr},
	{  0xC100 + 30, (char *) "CameraSettings:0x001E", (char *) nullptr},
	{  0xC100 + 31, (char *) "CameraSettings:0x001F", (char *) nullptr},
    {  0xC100 + 32, (char *) "CameraSettings:FocusContinuous", (char *) nullptr},
    {  0xC100 + 33, (char *) "CameraSettings:AESetting", (char *) nullptr},
    {  0xC100 + 34, (char *) "CameraSettings:ImageStabilization", (char *) nullptr},
    {  0xC100 + 35, (char *) "CameraSettings:DisplayAperture", (char *) nullptr},
    {  0xC100 + 36, (char *) "CameraSettings:ZoomSourceWidth", (char *) nullptr},
    {  0xC100 + 37, (char *) "CameraSettings:ZoomTargetWidth", (char *) nullptr},
	{  0xC100 + 38, (char *) "CameraSettings:0x0026", (char *) nullptr},
    {  0xC100 + 39, (char *) "CameraSettings:SpotMeteringMode", (char *) nullptr},
    {  0xC100 + 40, (char *) "CameraSettings:PhotoEffect", (char *) nullptr},
    {  0xC100 + 41, (char *) "CameraSettings:ManualFlashOutput", (char *) nullptr},
    {  0xC100 + 42, (char *) "CameraSettings:ColorTone", (char *) nullptr},
	{  0xC100 + 43, (char *) "CameraSettings:0x002B", (char *) nullptr},
	{  0xC100 + 44, (char *) "CameraSettings:0x002C", (char *) nullptr},
	{  0xC100 + 45, (char *) "CameraSettings:0x002D", (char *) nullptr},
    {  0xC100 + 46, (char *) "CameraSettings:SRAWQuality", (char *) nullptr},
	{  0xC100 + 47, (char *) "CameraSettings:0x002F", (char *) nullptr},
	{  0xC100 + 48, (char *) "CameraSettings:0x0030", (char *) nullptr},

	// Fields under tag 0x0002 (we add 0xC200 to make unique tag id)
    {  0xC200 + 0, (char *) "FocalLength:FocalType", (char *) nullptr},
    {  0xC200 + 1, (char *) "FocalLength:FocalLength", (char *) nullptr},
    {  0xC200 + 2, (char *) "FocalLength:FocalPlaneXSize", (char *) nullptr},
    {  0xC200 + 3, (char *) "FocalLength:FocalPlaneYSize", (char *) nullptr},
	
	// Fields under tag 0x0004 (we add 0xC400 to make unique tag id)
    {  0xC400 + 1, (char *) "ShotInfo:AutoISO", (char *) nullptr},
	{  0xC400 + 2, (char *) "ShotInfo:BaseISO", (char *) nullptr},
	{  0xC400 + 3, (char *) "ShotInfo:MeasuredEV", (char *) nullptr},
    {  0xC400 + 4, (char *) "ShotInfo:TargetAperture", (char *) nullptr},
    {  0xC400 + 5, (char *) "ShotInfo:TargetExposureTime", (char *) nullptr},
    {  0xC400 + 6, (char *) "ShotInfo:ExposureCompensation", (char *) nullptr},
    {  0xC400 + 7, (char *) "ShotInfo:WhiteBalance", (char *) nullptr},
    {  0xC400 + 8, (char *) "ShotInfo:SlowShutter", (char *) nullptr},
    {  0xC400 + 9, (char *) "ShotInfo:SequenceNumber", (char *) nullptr},
    {  0xC400 + 10, (char *) "ShotInfo:OpticalZoomCode", (char *) nullptr},
	{  0xC400 + 11, (char *) "ShotInfo:0x000B", (char *) nullptr},
	{  0xC400 + 12, (char *) "ShotInfo:CameraTemperature", (char *) nullptr},
    {  0xC400 + 13, (char *) "ShotInfo:FlashGuideNumber", (char *) nullptr},
    {  0xC400 + 14, (char *) "ShotInfo:AFPointsInFocus", (char *) nullptr},
    {  0xC400 + 15, (char *) "ShotInfo:FlashExposureComp", (char *) nullptr},
    {  0xC400 + 16, (char *) "ShotInfo:AutoExposureBracketing", (char *) nullptr},
    {  0xC400 + 17, (char *) "ShotInfo:AEBBracketValue", (char *) nullptr},
    {  0xC400 + 18, (char *) "ShotInfo:ControlMode", (char *) nullptr},
    {  0xC400 + 19, (char *) "ShotInfo:FocusDistanceUpper", (char *) nullptr},
    {  0xC400 + 20, (char *) "ShotInfo:FocusDistanceLower", (char *) nullptr},
    {  0xC400 + 21, (char *) "ShotInfo:FNumber", (char *) nullptr},
    {  0xC400 + 22, (char *) "ShotInfo:ExposureTime", (char *) nullptr},
    {  0xC400 + 23, (char *) "ShotInfo:MeasuredEV2", (char *) nullptr},
    {  0xC400 + 24, (char *) "ShotInfo:BulbDuration", (char *) nullptr},
	{  0xC400 + 25, (char *) "ShotInfo:0x0019", (char *) nullptr},
    {  0xC400 + 26, (char *) "ShotInfo:CameraType", (char *) nullptr},
    {  0xC400 + 27, (char *) "ShotInfo:AutoRotate", (char *) nullptr},
    {  0xC400 + 28, (char *) "ShotInfo:NDFilter", (char *) nullptr},
    {  0xC400 + 29, (char *) "ShotInfo:SelfTimer2", (char *) nullptr},
	{  0xC400 + 30, (char *) "ShotInfo:0x001E", (char *) nullptr},
	{  0xC400 + 31, (char *) "ShotInfo:0x001F", (char *) nullptr},
	{  0xC400 + 32, (char *) "ShotInfo:0x0020", (char *) nullptr},
    {  0xC400 + 33, (char *) "ShotInfo:FlashOutput", (char *) nullptr},

	// Fields under tag 0x0012 (we add 0x1200 to make unique tag id)
    {  0x1200 + 0, (char *) "AFInfo:NumAFPoints", (char *) nullptr},
    {  0x1200 + 1, (char *) "AFInfo:ValidAFPoints", (char *) nullptr},
    {  0x1200 + 2, (char *) "AFInfo:CanonImageWidth", (char *) nullptr},
    {  0x1200 + 3, (char *) "AFInfo:CanonImageHeight", (char *) nullptr},
    {  0x1200 + 4, (char *) "AFInfo:AFImageWidth", (char *) nullptr},
    {  0x1200 + 5, (char *) "AFInfo:AFImageHeight", (char *) nullptr},
    {  0x1200 + 6, (char *) "AFInfo:AFAreaWidth", (char *) nullptr},
    {  0x1200 + 7, (char *) "AFInfo:AFAreaHeight", (char *) nullptr},
    {  0x1200 + 8, (char *) "AFInfo:AFAreaXPositions", (char *) nullptr},
    {  0x1200 + 9, (char *) "AFInfo:AFAreaYPositions", (char *) nullptr},
    {  0x1200 + 10, (char *) "AFInfo:AFPointsInFocus", (char *) nullptr},
    {  0x1200 + 11, (char *) "AFInfo:PrimaryAFPoint?", (char *) nullptr},
    {  0x1200 + 12, (char *) "AFInfo:PrimaryAFPoint", (char *) nullptr},
	{  0x1200 + 13, (char *) "AFInfo:0x000D", (char *) nullptr},
	{  0x1200 + 14, (char *) "AFInfo:0x000E", (char *) nullptr},
	{  0x1200 + 15, (char *) "AFInfo:0x000F", (char *) nullptr},
	{  0x1200 + 16, (char *) "AFInfo:0x0010", (char *) nullptr},
	{  0x1200 + 17, (char *) "AFInfo:0x0011", (char *) nullptr},
	{  0x1200 + 18, (char *) "AFInfo:0x0012", (char *) nullptr},
	{  0x1200 + 19, (char *) "AFInfo:0x0013", (char *) nullptr},
	{  0x1200 + 20, (char *) "AFInfo:0x0014", (char *) nullptr},
	{  0x1200 + 21, (char *) "AFInfo:0x0015", (char *) nullptr},
	{  0x1200 + 22, (char *) "AFInfo:0x0016", (char *) nullptr},
	{  0x1200 + 23, (char *) "AFInfo:0x0017", (char *) nullptr},
	{  0x1200 + 24, (char *) "AFInfo:0x0018", (char *) nullptr},
	{  0x1200 + 25, (char *) "AFInfo:0x0019", (char *) nullptr},
	{  0x1200 + 26, (char *) "AFInfo:0x001A", (char *) nullptr},
	{  0x1200 + 27, (char *) "AFInfo:0x001B", (char *) nullptr},

	// Fields under tag 0x00A0 (we add 0xCA00 to make unique tag id)
    {  0xCA00 + 1, (char *) "ProcessingInfo:ToneCurve", (char *) nullptr},
    {  0xCA00 + 2, (char *) "ProcessingInfo:Sharpness", (char *) nullptr},
    {  0xCA00 + 3, (char *) "ProcessingInfo:SharpnessFrequency", (char *) nullptr},
    {  0xCA00 + 4, (char *) "ProcessingInfo:SensorRedLevel", (char *) nullptr},
    {  0xCA00 + 5, (char *) "ProcessingInfo:SensorBlueLevel", (char *) nullptr},
    {  0xCA00 + 6, (char *) "ProcessingInfo:WhiteBalanceRed", (char *) nullptr},
    {  0xCA00 + 7, (char *) "ProcessingInfo:WhiteBalanceBlue", (char *) nullptr},
    {  0xCA00 + 8, (char *) "ProcessingInfo:WhiteBalance", (char *) nullptr},
    {  0xCA00 + 9, (char *) "ProcessingInfo:ColorTemperature", (char *) nullptr},
    {  0xCA00 + 10, (char *) "ProcessingInfo:PictureStyle", (char *) nullptr},
    {  0xCA00 + 11, (char *) "ProcessingInfo:DigitalGain", (char *) nullptr},
    {  0xCA00 + 12, (char *) "ProcessingInfo:WBShiftAB", (char *) nullptr},
    {  0xCA00 + 13, (char *) "ProcessingInfo:WBShiftGM", (char *) nullptr},

	// Fields under tag 0x00E0 (we add 0xCE00 to make unique tag id)
    {  0xCE00 + 1, (char *) "SensorInfo:SensorWidth", (char *) nullptr},
    {  0xCE00 + 2, (char *) "SensorInfo:SensorHeight", (char *) nullptr},
	{  0xCE00 + 3, (char *) "SensorInfo:0x0003", (char *) nullptr},
	{  0xCE00 + 4, (char *) "SensorInfo:0x0004", (char *) nullptr},
    {  0xCE00 + 5, (char *) "SensorInfo:SensorLeftBorder", (char *) nullptr},
    {  0xCE00 + 6, (char *) "SensorInfo:SensorTopBorder", (char *) nullptr},
    {  0xCE00 + 7, (char *) "SensorInfo:SensorRightBorder", (char *) nullptr},
    {  0xCE00 + 8, (char *) "SensorInfo:SensorBottomBorder", (char *) nullptr},
    {  0xCE00 + 9, (char *) "SensorInfo:BlackMaskLeftBorder", (char *) nullptr},
    {  0xCE00 + 10, (char *) "SensorInfo:BlackMaskTopBorder", (char *) nullptr},
    {  0xCE00 + 11, (char *) "SensorInfo:BlackMaskRightBorder", (char *) nullptr},
    {  0xCE00 + 12, (char *) "SensorInfo:BlackMaskBottomBorder", (char *) nullptr},
	{  0xCE00 + 13, (char *) "SensorInfo:0x000D", (char *) nullptr},
	{  0xCE00 + 14, (char *) "SensorInfo:0x000E", (char *) nullptr},
	{  0xCE00 + 15, (char *) "SensorInfo:0x000F", (char *) nullptr},
	{  0xCE00 + 16, (char *) "SensorInfo:0x0010", (char *) nullptr},

    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Casio type 1 maker note
*/
static TagInfo
  exif_casio_type1_tag_table[] =
  {
    {  0x0001, (char *) "RecordingMode", (char *) nullptr},
    {  0x0002, (char *) "Quality", (char *) nullptr},
    {  0x0003, (char *) "FocusMode", (char *) nullptr},
    {  0x0004, (char *) "FlashMode", (char *) nullptr},
    {  0x0005, (char *) "FlashIntensity", (char *) nullptr},
    {  0x0006, (char *) "ObjectDistance", (char *) nullptr},
    {  0x0007, (char *) "WhiteBalance", (char *) nullptr},
    {  0x000A, (char *) "DigitalZoom", (char *) nullptr},
    {  0x000B, (char *) "Sharpness", (char *) nullptr},
    {  0x000C, (char *) "Contrast", (char *) nullptr},
    {  0x000D, (char *) "Saturation", (char *) nullptr},
    {  0x0014, (char *) "ISO", (char *) nullptr},
	{  0x0015, (char *) "FirmwareDate", (char *) nullptr},
    {  0x0016, (char *) "Enhancement", (char *) nullptr},
    {  0x0017, (char *) "ColorFilter", (char *) nullptr},
    {  0x0018, (char *) "AFPoint", (char *) nullptr},
    {  0x0019, (char *) "FlashIntensity", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Casio type 2 maker note
*/
static TagInfo
  exif_casio_type2_tag_table[] =
  {
    {  0x0002, (char *) "PreviewImageSize", (char *) nullptr},
    {  0x0003, (char *) "PreviewImageLength", (char *) nullptr},
    {  0x0004, (char *) "PreviewImageStart", (char *) nullptr},
    {  0x0008, (char *) "QualityMode", (char *) nullptr},
    {  0x0009, (char *) "CasioImageSize", (char *) nullptr},
    {  0x000D, (char *) "FocusMode", (char *) nullptr},
    {  0x0014, (char *) "ISO", (char *) nullptr},
    {  0x0019, (char *) "WhiteBalance", (char *) nullptr},
    {  0x001D, (char *) "FocalLength", (char *) nullptr},
    {  0x001F, (char *) "Saturation", (char *) nullptr},
    {  0x0020, (char *) "Contrast", (char *) nullptr},
    {  0x0021, (char *) "Sharpness", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x2000, (char *) "PreviewImage", (char *) nullptr},
    {  0x2001, (char *) "FirmwareDate", (char *) nullptr},
    {  0x2011, (char *) "WhiteBalanceBias", (char *) nullptr},
    {  0x2012, (char *) "WhiteBalance", (char *) nullptr},
    {  0x2021, (char *) "AFPointPosition", (char *) nullptr},
    {  0x2022, (char *) "ObjectDistance", (char *) nullptr},
    {  0x2034, (char *) "FlashDistance", (char *) nullptr},
	{  0x2076, (char *) "SpecialEffectMode", (char *) nullptr},
    {  0x3000, (char *) "RecordMode", (char *) nullptr},
    {  0x3001, (char *) "ReleaseMode", (char *) nullptr},
    {  0x3002, (char *) "Quality", (char *) nullptr},
    {  0x3003, (char *) "FocusMode", (char *) nullptr},
    {  0x3006, (char *) "HometownCity", (char *) nullptr},
    {  0x3007, (char *) "BestShotMode", (char *) nullptr},
    {  0x3008, (char *) "AutoISO", (char *) nullptr},
    {  0x3009, (char *) "AFMode", (char *) nullptr},
    {  0x3011, (char *) "Sharpness", (char *) nullptr},
    {  0x3012, (char *) "Contrast", (char *) nullptr},
    {  0x3013, (char *) "Saturation", (char *) nullptr},
    {  0x3014, (char *) "ISO", (char *) nullptr},
    {  0x3015, (char *) "ColorMode", (char *) nullptr},
    {  0x3016, (char *) "Enhancement", (char *) nullptr},
    {  0x3017, (char *) "ColorFilter", (char *) nullptr},
	{  0x301B, (char *) "ArtMode", (char *) nullptr},
    {  0x301C, (char *) "SequenceNumber", (char *) nullptr},
    {  0x301D, (char *) "BracketSequence", (char *) nullptr},
    {  0x3020, (char *) "ImageStabilization", (char *) nullptr},
    {  0x302A, (char *) "LightingMode", (char *) nullptr},
    {  0x302B, (char *) "PortraitRefiner", (char *) nullptr},
    {  0x3030, (char *) "SpecialEffectLevel", (char *) nullptr},
    {  0x3031, (char *) "SpecialEffectSetting", (char *) nullptr},
	{  0x3103, (char *) "DriveMode", (char *) nullptr},
	{  0x4001, (char *) "CaptureFrameRate", (char *) nullptr},
	{  0x4003, (char *) "VideoQuality", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
FujiFilm maker note
*/
static TagInfo
  exif_fujifilm_tag_table[] =
  {
    {  0x0000, (char *) "Version", (char *) nullptr},
    {  0x0010, (char *) "InternalSerialNumber", (char *) nullptr},
    {  0x1000, (char *) "Quality", (char *) nullptr},
    {  0x1001, (char *) "Sharpness", (char *) nullptr},
    {  0x1002, (char *) "WhiteBalance", (char *) nullptr},
    {  0x1003, (char *) "Saturation", (char *) nullptr},
    {  0x1004, (char *) "Contrast", (char *) nullptr},
	{  0x1005, (char *) "ColorTemperature", (char *) nullptr},
	{  0x100A, (char *) "WhiteBalanceFineTune", (char *) nullptr},
	{  0x100B, (char *) "NoiseReduction", (char *) nullptr},
    {  0x1010, (char *) "FujiFlashMode", (char *) nullptr},
    {  0x1011, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x1020, (char *) "Macro", (char *) nullptr},
    {  0x1021, (char *) "FocusMode", (char *) nullptr},
    {  0x1023, (char *) "FocusPixel", (char *) nullptr},
    {  0x1030, (char *) "SlowSync", (char *) nullptr},
    {  0x1031, (char *) "PictureMode", (char *) nullptr},
	{  0x1033, (char *) "EXRAuto", (char *) nullptr},
	{  0x1034, (char *) "EXRMode", (char *) nullptr},
    {  0x1100, (char *) "AutoBracketting", (char *) nullptr},
    {  0x1101, (char *) "SequenceNumber", (char *) nullptr},
    {  0x1210, (char *) "ColorMode", (char *) nullptr},
    {  0x1300, (char *) "BlurWarning", (char *) nullptr},
    {  0x1301, (char *) "FocusWarning", (char *) nullptr},
    {  0x1302, (char *) "ExposureWarning", (char *) nullptr},
    {  0x1400, (char *) "DynamicRange", (char *) nullptr},
	{  0x1401, (char *) "FilmMode", (char *) nullptr},
	{  0x1402, (char *) "DynamicRangeSetting", (char *) nullptr},
	{  0x1403, (char *) "DevelopmentDynamicRange", (char *) nullptr},
	{  0x1404, (char *) "MinFocalLength", (char *) nullptr},
	{  0x1405, (char *) "MaxFocalLength", (char *) nullptr},
	{  0x1406, (char *) "MaxApertureAtMinFocal", (char *) nullptr},
	{  0x1407, (char *) "MaxApertureAtMaxFocal", (char *) nullptr},
	{  0x4100, (char *) "FacesDetected", (char *) nullptr},
	{  0x4103, (char *) "FacePositions", (char *) nullptr},
	{  0x8000, (char *) "FileSource", (char *) nullptr},
	{  0x8002, (char *) "OrderNumber", (char *) nullptr},
	{  0x8003, (char *) "FrameNumber", (char *) nullptr},
	{  0xB211, (char *) "Parallax", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Kyocera maker note
*/
static TagInfo
  exif_kyocera_tag_table[] =
  {
    {  0x0001, (char *) "ThumbnailImage", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) "Print Image Matching Info"},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Olympus Type 1 / Epson / Agfa maker note
*/
static TagInfo
  exif_olympus_type1_tag_table[] =
  {
    {  0x0000, (char *) "MakerNoteVersion", (char *) nullptr},
    {  0x0001, (char *) "MinoltaCameraSettingsOld", (char *) nullptr},
    {  0x0003, (char *) "MinoltaCameraSettings", (char *) nullptr},
    {  0x0040, (char *) "CompressedImageSize", (char *) nullptr},
    {  0x0081, (char *) "PreviewImageData", (char *) nullptr},
    {  0x0088, (char *) "PreviewImageStart", (char *) nullptr},
    {  0x0089, (char *) "PreviewImageLength", (char *) nullptr},
    {  0x0100, (char *) "ThumbnailImage", (char *) nullptr},
    {  0x0104, (char *) "BodyFirmwareVersion", (char *) nullptr},
    {  0x0200, (char *) "SpecialMode", (char *) nullptr},
    {  0x0201, (char *) "Quality", (char *) nullptr},
    {  0x0202, (char *) "Macro", (char *) nullptr},
    {  0x0203, (char *) "BWMode", (char *) nullptr},
    {  0x0204, (char *) "DigitalZoom", (char *) nullptr},
    {  0x0205, (char *) "FocalPlaneDiagonal", (char *) nullptr},
	{  0x0206, (char *) "LensDistortionParams", (char *) nullptr},
    {  0x0207, (char *) "CameraType", (char *) nullptr},
    {  0x0208, (char *) "TextInfo", (char *) "Olympus TextInfo Tags"},
    {  0x0209, (char *) "CameraID", (char *) nullptr},
    {  0x020B, (char *) "EpsonImageWidth", (char *) nullptr},
    {  0x020C, (char *) "EpsonImageHeight", (char *) nullptr},
    {  0x020D, (char *) "EpsonSoftware", (char *) nullptr},
    {  0x0280, (char *) "PreviewImage", (char *) nullptr},
    {  0x0300, (char *) "PreCaptureFrames", (char *) nullptr},
	{  0x0301, (char *) "WhiteBoard", (char *) nullptr},
    {  0x0302, (char *) "OneTouchWB", (char *) nullptr}, 
	{  0x0303, (char *) "WhiteBalanceBracket", (char *) nullptr}, 
	{  0x0304, (char *) "WhiteBalanceBias", (char *) nullptr}, 
	{  0x0403, (char *) "SceneMode", (char *) nullptr}, 
    {  0x0404, (char *) "SerialNumber", (char *) nullptr}, 
	{  0x0405, (char *) "Firmware", (char *) nullptr}, 
    {  0x0E00, (char *) "PrintIM", (char *) "PrintIM Tags"}, 
	{  0x0F00, (char *) "DataDump", (char *) nullptr},
	{  0x0F01, (char *) "DataDump2", (char *) nullptr},
	{  0x1000, (char *) "ShutterSpeedValue", (char *) nullptr},
	{  0x1001, (char *) "ISOValue", (char *) nullptr},
	{  0x1002, (char *) "ApertureValue", (char *) nullptr},
	{  0x1003, (char *) "BrightnessValue", (char *) nullptr},
	{  0x1004, (char *) "FlashMode", (char *) nullptr},
	{  0x1005, (char *) "FlashDevice", (char *) nullptr},
	{  0x1006, (char *) "ExposureCompensation", (char *) nullptr},
	{  0x1007, (char *) "SensorTemperature", (char *) nullptr},
	{  0x1008, (char *) "LensTemperature", (char *) nullptr},
	{  0x1009, (char *) "LightCondition", (char *) nullptr},
	{  0x100A, (char *) "FocusRange", (char *) nullptr},
	{  0x100B, (char *) "FocusMode", (char *) nullptr},
	{  0x100C, (char *) "ManualFocusDistance", (char *) nullptr},
	{  0x100D, (char *) "ZoomStepCount", (char *) nullptr},
	{  0x100E, (char *) "FocusStepCount", (char *) nullptr},
	{  0x100F, (char *) "Sharpness", (char *) nullptr},
	{  0x1010, (char *) "FlashChargeLevel", (char *) nullptr},
	{  0x1011, (char *) "ColorMatrix", (char *) nullptr},
	{  0x1012, (char *) "BlackLevel", (char *) nullptr},
	{  0x1015, (char *) "WBMode", (char *) nullptr},
	{  0x1017, (char *) "RedBalance", (char *) nullptr},
	{  0x1018, (char *) "BlueBalance", (char *) nullptr},
	{  0x1019, (char *) "ColorMatrixNumber", (char *) nullptr},
	{  0x101A, (char *) "SerialNumber", (char *) nullptr},
	{  0x1023, (char *) "FlashExposureComp", (char *) nullptr},
	{  0x1024, (char *) "InternalFlashTable", (char *) nullptr},
	{  0x1025, (char *) "ExternalFlashGValue", (char *) nullptr},
	{  0x1026, (char *) "ExternalFlashBounce", (char *) nullptr},
	{  0x1027, (char *) "ExternalFlashZoom", (char *) nullptr},
	{  0x1028, (char *) "ExternalFlashMode", (char *) nullptr},
	{  0x1029, (char *) "Contrast", (char *) nullptr},
	{  0x102A, (char *) "SharpnessFactor", (char *) nullptr},
	{  0x102B, (char *) "ColorControl", (char *) nullptr},
	{  0x102C, (char *) "ValidBits", (char *) nullptr},
	{  0x102D, (char *) "CoringFilter", (char *) nullptr},
	{  0x102E, (char *) "OlympusImageWidth", (char *) nullptr},
	{  0x102F, (char *) "OlympusImageHeight", (char *) nullptr},
	{  0x1030, (char *) "SceneDetect", (char *) nullptr},
	{  0x1031, (char *) "SceneArea?", (char *) nullptr},
	{  0x1033, (char *) "SceneDetectData?", (char *) nullptr},
	{  0x1034, (char *) "CompressionRatio", (char *) nullptr},
	{  0x1035, (char *) "PreviewImageValid", (char *) nullptr},
	{  0x1036, (char *) "PreviewImageStart", (char *) nullptr},
	{  0x1037, (char *) "PreviewImageLength", (char *) nullptr},
	{  0x1038, (char *) "AFResult", (char *) nullptr},
	{  0x1039, (char *) "CCDScanMode", (char *) nullptr},
	{  0x103A, (char *) "NoiseReduction", (char *) nullptr},
	{  0x103B, (char *) "InfinityLensStep", (char *) nullptr},
	{  0x103C, (char *) "NearLensStep", (char *) nullptr},
	{  0x103D, (char *) "LightValueCenter", (char *) nullptr},
	{  0x103E, (char *) "LightValuePeriphery", (char *) nullptr},
	{  0x2010, (char *) "Equipment", (char *) "Olympus Equipment Tags"},
	{  0x2020, (char *) "CameraSettings", (char *) "Olympus CameraSettings Tags"},
	{  0x2030, (char *) "RawDevelopment", (char *) "Olympus RawDevelopment Tags"},
	{  0x2040, (char *) "ImageProcessing", (char *) "Olympus ImageProcessing Tags"},
	{  0x2050, (char *) "FocusInfo", (char *) "Olympus FocusInfo Tags"},
	{  0x2100, (char *) "Olympus2100", (char *) "Olympus FE Tags"},
	{  0x2200, (char *) "Olympus2200", (char *) "Olympus FE Tags"},
	{  0x2300, (char *) "Olympus2300", (char *) "Olympus FE Tags"},
	{  0x2400, (char *) "Olympus2400", (char *) "Olympus FE Tags"},
	{  0x2500, (char *) "Olympus2500", (char *) "Olympus FE Tags"},
	{  0x2600, (char *) "Olympus2600", (char *) "Olympus FE Tags"},
	{  0x2700, (char *) "Olympus2700", (char *) "Olympus FE Tags"},
	{  0x2800, (char *) "Olympus2800", (char *) "Olympus FE Tags"},
	{  0x2900, (char *) "Olympus2900", (char *) "Olympus FE Tags"},
	{  0x3000, (char *) "RawInfo", (char *) "Olympus RawInfo Tags"},
	{  0x4000, (char *) "MainInfo", (char *) "Olympus MainInfo Tags"},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Minolta maker note
*/
static TagInfo
  exif_minolta_tag_table[] =
  {
    {  0x0000, (char *) "MakerNoteVersion", (char *) nullptr},
    {  0x0001, (char *) "MinoltaCameraSettingsOld", (char *) nullptr},
    {  0x0003, (char *) "MinoltaCameraSettings", (char *) nullptr},
    {  0x0004, (char *) "MinoltaCameraSettings7D", (char *) nullptr},
    {  0x0018, (char *) "ImageStabilization", (char *) nullptr},
	{  0x0040, (char *) "CompressedImageSize", (char *) nullptr},
    {  0x0081, (char *) "PreviewImage", (char *) nullptr},
    {  0x0088, (char *) "PreviewImageStart", (char *) nullptr},
    {  0x0089, (char *) "PreviewImageLength", (char *) nullptr},
    {  0x0100, (char *) "SceneMode", (char *) nullptr},
    {  0x0101, (char *) "ColorMode", (char *) nullptr},
    {  0x0102, (char *) "MinoltaQuality", (char *) nullptr},
    {  0x0103, (char *) "MinoltaImageSize", (char *) nullptr},
    {  0x0104, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x0105, (char *) "Teleconverter", (char *) nullptr},
    {  0x0107, (char *) "ImageStabilization", (char *) nullptr},
    {  0x0109, (char *) "RawAndJpgRecording", (char *) nullptr},
    {  0x010A, (char *) "ZoneMatching", (char *) nullptr},
    {  0x010B, (char *) "ColorTemperature", (char *) nullptr},
    {  0x010C, (char *) "LensType", (char *) nullptr},
    {  0x0111, (char *) "ColorCompensationFilter", (char *) nullptr},
    {  0x0112, (char *) "WhiteBalanceFineTune", (char *) nullptr},
    {  0x0113, (char *) "ImageStabilization", (char *) nullptr},
    {  0x0114, (char *) "MinoltaCameraSettings5D", (char *) nullptr},
    {  0x0115, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x0F00, (char *) "MinoltaCameraSettings2", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
There are 3 formats of Nikon's MakerNote. MakerNote of E700/E800/E900/E900S/E910/E950
starts from ASCII string "Nikon". Data format is the same as IFD, but it starts from
offset 0x08. This is the same as Olympus except start string. 
*/

/**
TYPE 1 is for E-Series cameras prior to (not including) E990
*/
static TagInfo
  exif_nikon_type1_tag_table[] =
  {
    {  0x0002, (char *) "FamilyID", (char *) nullptr},
    {  0x0003, (char *) "Quality", (char *) nullptr},
    {  0x0004, (char *) "ColorMode", (char *) nullptr},
    {  0x0005, (char *) "ImageAdjustment", (char *) nullptr},
    {  0x0006, (char *) "CCDSensitivity", (char *) nullptr},
    {  0x0007, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0008, (char *) "Focus", (char *) nullptr},
    {  0x000A, (char *) "DigitalZoom", (char *) nullptr},
    {  0x000B, (char *) "FisheyeConverter", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Nikon type 2 maker note
*/
static TagInfo
  exif_nikon_type2_tag_table[] =
  {
    {  0x0001, (char *) "MakerNoteVersion", (char *) nullptr},
    {  0x0002, (char *) "ISO", (char *) nullptr},
    {  0x0003, (char *) "ColorMode", (char *) nullptr},
    {  0x0004, (char *) "Quality", (char *) nullptr},
    {  0x0005, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0006, (char *) "Sharpness", (char *) nullptr},
    {  0x0007, (char *) "FocusMode", (char *) nullptr},
    {  0x0008, (char *) "FlashSetting", (char *) nullptr},
    {  0x0009, (char *) "FlashType", (char *) nullptr},
    {  0x000B, (char *) "WhiteBalanceFineTune", (char *) nullptr},
    {  0x000F, (char *) "ISOSelection", (char *) nullptr},
    {  0x0010, (char *) "DataDump", (char *) nullptr},
    {  0x0080, (char *) "ImageAdjustment", (char *) nullptr},
    {  0x0082, (char *) "AuxiliaryLens", (char *) nullptr},
    {  0x0085, (char *) "ManualFocusDistance", (char *) nullptr},
    {  0x0086, (char *) "DigitalZoom", (char *) nullptr},
    {  0x0088, (char *) "AFInfo", (char *) nullptr},
    {  0x0089, (char *) "ShootingMode", (char *) nullptr},
    {  0x008D, (char *) "ColorMode", (char *) nullptr},
    {  0x008F, (char *) "SceneMode", (char *) nullptr},
    {  0x0092, (char *) "HueAdjustment", (char *) nullptr},
    {  0x0094, (char *) "Saturation", (char *) nullptr},
    {  0x0095, (char *) "NoiseReduction", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
The type-3 directory is for D-Series cameras such as the D1 and D100.
see http://www.timelesswanderings.net/equipment/D100/NEF.html
*/
static TagInfo
  exif_nikon_type3_tag_table[] =
  {
    {  0x0001, (char *) "MakerNoteVersion", (char *) nullptr},
    {  0x0002, (char *) "ISO", (char *) nullptr},
    {  0x0003, (char *) "ColorMode", (char *) nullptr},
    {  0x0004, (char *) "Quality", (char *) nullptr},
    {  0x0005, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0006, (char *) "Sharpness", (char *) nullptr},
    {  0x0007, (char *) "FocusMode", (char *) nullptr},
    {  0x0008, (char *) "FlashSetting", (char *) nullptr},
    {  0x0009, (char *) "FlashType", (char *) nullptr},
    {  0x000B, (char *) "WhiteBalanceFineTune", (char *) nullptr},
    {  0x000C, (char *) "WB_RBLevels", (char *) nullptr},
    {  0x000D, (char *) "ProgramShift", (char *) nullptr},
    {  0x000E, (char *) "ExposureDifference", (char *) nullptr},
    {  0x000F, (char *) "ISOSelection", (char *) nullptr},
	{  0x0010, (char *) "DataDump", (char *) nullptr},
    {  0x0011, (char *) "PreviewIFD", (char *) nullptr},
    {  0x0012, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x0013, (char *) "ISOSetting", (char *) nullptr},
	{  0x0014, (char *) "ColorBalanceA", (char *) nullptr},
    {  0x0016, (char *) "ImageBoundary", (char *) nullptr},
	{  0x0017, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x0018, (char *) "FlashExposureBracketValue", (char *) nullptr},
    {  0x0019, (char *) "ExposureBracketValue", (char *) nullptr},
    {  0x001A, (char *) "ImageProcessing", (char *) nullptr},
    {  0x001B, (char *) "CropHiSpeed", (char *) nullptr},
	{  0x001C, (char *) "ExposureTuning", (char *) nullptr},
    {  0x001D, (char *) "SerialNumber", (char *) nullptr},
    {  0x001E, (char *) "ColorSpace", (char *) nullptr},
	{  0x001F, (char *) "VRInfo", (char *) nullptr},
	{  0x0020, (char *) "ImageAuthentication", (char *) nullptr},
	{  0x0022, (char *) "ActiveD-Lighting", (char *) nullptr},
	{  0x0023, (char *) "PictureControl", (char *) nullptr},
	{  0x0024, (char *) "WorldTime", (char *) nullptr},
	{  0x0025, (char *) "ISOInfo", (char *) nullptr},
	{  0x002A, (char *) "VignetteControl", (char *) nullptr},
	{  0x002B, (char *) "DistortInfo", (char *) nullptr},
	{  0x0080, (char *) "ImageAdjustment", (char *) nullptr},
	{  0x0081, (char *) "ToneComp", (char *) nullptr},
	{  0x0082, (char *) "AuxiliaryLens", (char *) nullptr},
	{  0x0083, (char *) "LensType", (char *) nullptr},
    {  0x0084, (char *) "Lens", (char *) nullptr},
    {  0x0085, (char *) "ManualFocusDistance", (char *) nullptr},
    {  0x0086, (char *) "DigitalZoom", (char *) nullptr},
    {  0x0087, (char *) "FlashMode", (char *) nullptr},
    {  0x0088, (char *) "AFInfo", (char *) nullptr},
    {  0x0089, (char *) "ShootingMode", (char *) nullptr},
    {  0x008B, (char *) "LensFStops", (char *) nullptr},
    {  0x008C, (char *) "ContrastCurve", (char *) nullptr},
    {  0x008D, (char *) "ColorHue", (char *) nullptr},
    {  0x008F, (char *) "SceneMode", (char *) nullptr},
    {  0x0090, (char *) "LightSource", (char *) nullptr},
	{  0x0091, (char *) "ShotInfo", (char *) nullptr},
    {  0x0092, (char *) "HueAdjustment", (char *) nullptr},
	{  0x0093, (char *) "NEFCompression", (char *) nullptr},
    {  0x0094, (char *) "Saturation", (char *) nullptr},
    {  0x0095, (char *) "NoiseReduction", (char *) nullptr},
    {  0x0096, (char *) "LinearizationTable", (char *) nullptr},
	{  0x0097, (char *) "ColorBalance", (char *) nullptr},
	{  0x0098, (char *) "LensData", (char *) nullptr},
    {  0x0099, (char *) "RawImageCenter", (char *) nullptr},
    {  0x009A, (char *) "SensorPixelSize", (char *) nullptr},
    {  0x009C, (char *) "SceneAssist", (char *) nullptr},
    {  0x009E, (char *) "RetouchHistory", (char *) nullptr},
    {  0x00A0, (char *) "SerialNumber", (char *) nullptr},
    {  0x00A2, (char *) "ImageDataSize", (char *) nullptr},
    {  0x00A5, (char *) "ImageCount", (char *) nullptr},
    {  0x00A6, (char *) "DeletedImageCount", (char *) nullptr},
    {  0x00A7, (char *) "ShutterCount", (char *) nullptr},
	{  0x00A8, (char *) "FlashInfo", (char *) nullptr},
    {  0x00A9, (char *) "ImageOptimization", (char *) nullptr},
    {  0x00AA, (char *) "Saturation", (char *) nullptr},
    {  0x00AB, (char *) "VariProgram", (char *) nullptr},
    {  0x00AC, (char *) "ImageStabilization", (char *) nullptr},
    {  0x00AD, (char *) "AFResponse", (char *) nullptr},
    {  0x00B0, (char *) "MultiExposure", (char *) nullptr},
    {  0x00B1, (char *) "HighISONoiseReduction", (char *) nullptr},
    {  0x00B3, (char *) "ToningEffect", (char *) nullptr},
    {  0x00B6, (char *) "PowerUpTime", (char *) nullptr},
    {  0x00B7, (char *) "AFInfo2", (char *) nullptr},
    {  0x00B8, (char *) "FileInfo", (char *) nullptr},
    {  0x00B9, (char *) "AFTune", (char *) nullptr},
    {  0x00BD, (char *) "PictureControl", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x0E01, (char *) "NikonCaptureData", (char *) nullptr},
    {  0x0E09, (char *) "NikonCaptureVersion", (char *) nullptr},
    {  0x0E0E, (char *) "NikonCaptureOffsets", (char *) nullptr},
    {  0x0E10, (char *) "NikonScanIFD", (char *) nullptr},
    {  0x0E1D, (char *) "NikonICCProfile", (char *) nullptr},
    {  0x0E1E, (char *) "NikonCaptureOutput", (char *) nullptr},
	{  0x0E22, (char *) "NEFBitDepth", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Panasonic / Leica maker note
*/
static TagInfo
  exif_panasonic_tag_table[] =
  {
    {  0x0001, (char *) "ImageQuality", (char *) nullptr},
    {  0x0002, (char *) "FirmwareVersion", (char *) nullptr},
    {  0x0003, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0007, (char *) "FocusMode", (char *) nullptr},
    {  0x000F, (char *) "AFAreaMode", (char *) nullptr},
    {  0x001A, (char *) "ImageStabilization", (char *) nullptr},
    {  0x001C, (char *) "MacroMode", (char *) nullptr},
    {  0x001F, (char *) "ShootingMode", (char *) nullptr},
    {  0x0020, (char *) "Audio", (char *) nullptr},
    {  0x0021, (char *) "DataDump", (char *) nullptr},
	{  0x0022, (char *) "EasyMode", (char *) nullptr},
    {  0x0023, (char *) "WhiteBalanceBias", (char *) nullptr},
    {  0x0024, (char *) "FlashBias", (char *) nullptr},
    {  0x0025, (char *) "InternalSerialNumber", (char *) nullptr},
	{  0x0026, (char *) "PanasonicExifVersion", (char *) nullptr},
    {  0x0028, (char *) "ColorEffect", (char *) nullptr},
	{  0x0029, (char *) "TimeSincePowerOn", (char *) nullptr},
    {  0x002A, (char *) "BurstMode", (char *) nullptr},
    {  0x002B, (char *) "SequenceNumber", (char *) nullptr},
    {  0x002C, (char *) "ContrastMode", (char *) nullptr},
    {  0x002D, (char *) "NoiseReduction", (char *) nullptr},
    {  0x002E, (char *) "SelfTimer", (char *) nullptr},
    {  0x0030, (char *) "Rotation", (char *) nullptr},
	{  0x0031, (char *) "AFAssistLamp", (char *) nullptr},
    {  0x0032, (char *) "ColorMode", (char *) nullptr},
	{  0x0033, (char *) "BabyAge_0x0033", (char *) nullptr},
	{  0x0034, (char *) "OpticalZoomMode", (char *) nullptr},
	{  0x0035, (char *) "ConversionLens", (char *) nullptr},
	{  0x0036, (char *) "TravelDay", (char *) nullptr},
	{  0x0039, (char *) "Contrast", (char *) nullptr},
	{  0x003A, (char *) "WorldTimeLocation", (char *) nullptr},
	{  0x003B, (char *) "TextStamp_0x003B", (char *) nullptr},
	{  0x003C, (char *) "ProgramISO", (char *) nullptr},
	{  0x003D, (char *) "AdvancedSceneMode", (char *) nullptr},
	{  0x003E, (char *) "TextStamp_0x003E", (char *) nullptr},
	{  0x003F, (char *) "FacesDetected", (char *) nullptr},
	{  0x0040, (char *) "Saturation", (char *) nullptr},
	{  0x0041, (char *) "Sharpness", (char *) nullptr},
	{  0x0042, (char *) "FilmMode", (char *) nullptr},
	{  0x0046, (char *) "WBAdjustAB", (char *) nullptr},
	{  0x0047, (char *) "WBAdjustGM", (char *) nullptr},
	{  0x004B, (char *) "PanasonicImageWidth", (char *) nullptr},
	{  0x004C, (char *) "PanasonicImageHeight", (char *) nullptr},
	{  0x004D, (char *) "AFPointPosition", (char *) nullptr},
	{  0x004E, (char *) "FaceDetInfo", (char *) "Panasonic FaceDetInfo Tags"},
	{  0x0051, (char *) "LensType", (char *) nullptr},
	{  0x0052, (char *) "LensSerialNumber", (char *) nullptr},
	{  0x0053, (char *) "AccessoryType", (char *) nullptr},
	{  0x0059, (char *) "Transform", (char *) nullptr},
	{  0x005D, (char *) "IntelligentExposure", (char *) nullptr},
	{  0x0061, (char *) "FaceRecInfo", (char *) "Panasonic FaceRecInfo Tags"},
	{  0x0062, (char *) "FlashWarning", (char *) nullptr},
	{  0x0063, (char *) "RecognizedFaceFlags?", (char *) nullptr},
	{  0x0065, (char *) "Title", (char *) nullptr},
	{  0x0066, (char *) "BabyName", (char *) nullptr},
	{  0x0067, (char *) "Location", (char *) nullptr},
	{  0x0069, (char *) "Country", (char *) nullptr},
	{  0x006B, (char *) "State", (char *) nullptr},
	{  0x006D, (char *) "City", (char *) nullptr},
	{  0x006F, (char *) "Landmark", (char *) nullptr},
	{  0x0070, (char *) "IntelligentResolution", (char *) nullptr},
	{  0x0079, (char *) "IntelligentD-Range", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
	{  0x8000, (char *) "MakerNoteVersion", (char *) nullptr},
	{  0x8001, (char *) "SceneMode", (char *) nullptr},
	{  0x8004, (char *) "WBRedLevel", (char *) nullptr},
	{  0x8005, (char *) "WBGreenLevel", (char *) nullptr},
	{  0x8006, (char *) "WBBlueLevel", (char *) nullptr},
	{  0x8007, (char *) "FlashFired", (char *) nullptr},
	{  0x8008, (char *) "TextStamp_0x8008", (char *) nullptr},
	{  0x8009, (char *) "TextStamp_0x8009", (char *) nullptr},
	{  0x8010, (char *) "BabyAge_0x8010", (char *) nullptr},
	{  0x8012, (char *) "Transform", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Pentax (Asahi) maker note type 1
*/
static TagInfo
  exif_asahi_tag_table[] =
  {
    {  0x0001, (char *) "Capture Mode", (char *) nullptr},
    {  0x0002, (char *) "Quality Level", (char *) nullptr},
    {  0x0003, (char *) "Focus Mode", (char *) nullptr},
    {  0x0004, (char *) "Flash Mode", (char *) nullptr},
    {  0x0007, (char *) "White Balance", (char *) nullptr},
    {  0x000A, (char *) "Digital Zoom", (char *) nullptr},
    {  0x000B, (char *) "Sharpness", (char *) nullptr},
    {  0x000C, (char *) "Contrast", (char *) nullptr},
    {  0x000D, (char *) "Saturation", (char *) nullptr},
    {  0x0014, (char *) "ISO Speed", (char *) nullptr},
    {  0x0017, (char *) "Color", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x1000, (char *) "Time Zone", (char *) nullptr},
    {  0x1001, (char *) "Daylight Savings", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Pentax maker note type 2
*/
static TagInfo
  exif_pentax_tag_table[] =
  {
    {  0x0000, (char *) "PentaxVersion", (char *) nullptr},
    {  0x0001, (char *) "PentaxMode", (char *) nullptr},
    {  0x0002, (char *) "PreviewImageSize", (char *) nullptr},
    {  0x0003, (char *) "PreviewImageLength", (char *) nullptr},
    {  0x0004, (char *) "PreviewImageStart", (char *) nullptr},
    {  0x0005, (char *) "PentaxModelID", (char *) "Pentax PentaxModelID Values"},
    {  0x0006, (char *) "Date", (char *) nullptr},
    {  0x0007, (char *) "Time", (char *) nullptr},
    {  0x0008, (char *) "Quality", (char *) nullptr},
    {  0x0009, (char *) "PentaxImageSize", (char *) nullptr},
    {  0x000B, (char *) "PictureMode", (char *) nullptr},
    {  0x000C, (char *) "FlashMode", (char *) nullptr},
    {  0x000D, (char *) "FocusMode", (char *) nullptr},
    {  0x000E, (char *) "AFPointSelected", (char *) nullptr},
    {  0x000F, (char *) "AFPointsInFocus", (char *) nullptr},
    {  0x0010, (char *) "FocusPosition", (char *) nullptr},
    {  0x0012, (char *) "ExposureTime", (char *) nullptr},
    {  0x0013, (char *) "FNumber", (char *) nullptr},
    {  0x0014, (char *) "ISO", (char *) nullptr},
	{  0x0015, (char *) "LightReading", (char *) nullptr},
    {  0x0016, (char *) "ExposureCompensation", (char *) nullptr},
    {  0x0017, (char *) "MeteringMode", (char *) nullptr},
    {  0x0018, (char *) "AutoBracketing", (char *) nullptr},
    {  0x0019, (char *) "WhiteBalance", (char *) nullptr},
    {  0x001A, (char *) "WhiteBalanceMode", (char *) nullptr},
    {  0x001B, (char *) "BlueBalance", (char *) nullptr},
    {  0x001C, (char *) "RedBalance", (char *) nullptr},
    {  0x001D, (char *) "FocalLength", (char *) nullptr},
    {  0x001E, (char *) "DigitalZoom", (char *) nullptr},
    {  0x001F, (char *) "Saturation", (char *) nullptr},
    {  0x0020, (char *) "Contrast", (char *) nullptr},
    {  0x0021, (char *) "Sharpness", (char *) nullptr},
    {  0x0022, (char *) "WorldTimeLocation", (char *) nullptr},
    {  0x0023, (char *) "HometownCity", (char *) "Pentax City Values"},
    {  0x0024, (char *) "DestinationCity", (char *) "Pentax City Values"},
    {  0x0025, (char *) "HometownDST", (char *) nullptr},
    {  0x0026, (char *) "DestinationDST", (char *) nullptr},
    {  0x0027, (char *) "DSPFirmwareVersion", (char *) nullptr},
    {  0x0028, (char *) "CPUFirmwareVersion", (char *) nullptr},
    {  0x0029, (char *) "FrameNumber", (char *) nullptr},
    {  0x002D, (char *) "EffectiveLV", (char *) nullptr},
    {  0x0032, (char *) "ImageProcessing", (char *) nullptr},
    {  0x0033, (char *) "PictureMode", (char *) nullptr},
    {  0x0034, (char *) "DriveMode", (char *) nullptr},
	{  0x0035, (char *) "SensorSize", (char *) nullptr},
    {  0x0037, (char *) "ColorSpace", (char *) nullptr},
    {  0x0039, (char *) "RawImageSize", (char *) nullptr},
	{  0x003C, (char *) "AFPointsInFocus", (char *) nullptr},
    {  0x003E, (char *) "PreviewImageBorders", (char *) nullptr},
    {  0x003F, (char *) "LensType", (char *) "Pentax LensType Values"},
    {  0x0040, (char *) "SensitivityAdjust", (char *) nullptr},
    {  0x0041, (char *) "ImageProcessingCount", (char *) nullptr},
    {  0x0047, (char *) "CameraTemperature", (char *) nullptr},
    {  0x0048, (char *) "AELock", (char *) nullptr},
    {  0x0049, (char *) "NoiseReduction", (char *) nullptr},
    {  0x004D, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x004F, (char *) "ImageTone", (char *) nullptr},
	{  0x0050, (char *) "ColorTemperature", (char *) nullptr},
    {  0x005C, (char *) "ShakeReductionInfo", (char *) "Pentax SRInfo Tags"},
    {  0x005D, (char *) "ShutterCount", (char *) nullptr},
	{  0x0060, (char *) "FaceInfo", (char *) "Pentax FaceInfo Tags"},
	{  0x0067, (char *) "Hue", (char *) nullptr},
	{  0x0068, (char *) "AWBInfo", (char *) "Pentax AWBInfo Tags"},
    {  0x0069, (char *) "DynamicRangeExpansion", (char *) nullptr},
	{  0x006B, (char *) "TimeInfo", (char *) "Pentax TimeInfo Tags"},
	{  0x006C, (char *) "HighLowKeyAdj", (char *) nullptr},
	{  0x006D, (char *) "ContrastHighlight", (char *) nullptr},
	{  0x006E, (char *) "ContrastShadow", (char *) nullptr},
	{  0x006F, (char *) "ContrastHighlightShadowAdj", (char *) nullptr},
	{  0x0070, (char *) "FineSharpness", (char *) nullptr},
    {  0x0071, (char *) "HighISONoiseReduction", (char *) nullptr},
    {  0x0072, (char *) "AFAdjustment", (char *) nullptr},
	{  0x0073, (char *) "MonochromeFilterEffect", (char *) nullptr},
	{  0x0074, (char *) "MonochromeToning", (char *) nullptr},
	{  0x0076, (char *) "FaceDetect", (char *) nullptr},
	{  0x0077, (char *) "FaceDetectFrameSize", (char *) nullptr},
	{  0x0079, (char *) "ShadowCompensation", (char *) nullptr},
	{  0x007A, (char *) "ISOAutoParameters", (char *) nullptr},
	{  0x007B, (char *) "CrossProcess", (char *) nullptr},
	{  0x007D, (char *) "LensCorr", (char *) "Pentax LensCorr Tags"},
	{  0x007F, (char *) "BleachBypassToning", (char *) nullptr},
    {  0x0200, (char *) "BlackPoint", (char *) nullptr},
    {  0x0201, (char *) "WhitePoint", (char *) nullptr},
    {  0x0203, (char *) "ColorMatrixA", (char *) nullptr},
    {  0x0204, (char *) "ColorMatrixB", (char *) nullptr},
    {  0x0205, (char *) "CameraSettings", (char *) "Pentax CameraSettings Tags"},
	{  0x0206, (char *) "AEInfo", (char *) "Pentax AEInfo Tags"},
    {  0x0207, (char *) "LensInfo", (char *) "Pentax LensInfo Tags"},
    {  0x0208, (char *) "FlashInfo", (char *) "Pentax FlashInfo Tags"},
    {  0x0209, (char *) "AEMeteringSegments", (char *) nullptr},
    {  0x020A, (char *) "FlashMeteringSegments", (char *) nullptr},
    {  0x020B, (char *) "SlaveFlashMeteringSegments", (char *) nullptr},
    {  0x020D, (char *) "WB_RGGBLevelsDaylight", (char *) nullptr},
    {  0x020E, (char *) "WB_RGGBLevelsShade", (char *) nullptr},
    {  0x020F, (char *) "WB_RGGBLevelsCloudy", (char *) nullptr},
    {  0x0210, (char *) "WB_RGGBLevelsTungsten", (char *) nullptr},
    {  0x0211, (char *) "WB_RGGBLevelsFluorescentD", (char *) nullptr},
    {  0x0212, (char *) "WB_RGGBLevelsFluorescentN", (char *) nullptr},
    {  0x0213, (char *) "WB_RGGBLevelsFluorescentW", (char *) nullptr},
    {  0x0214, (char *) "WB_RGGBLevelsFlash", (char *) nullptr},
    {  0x0215, (char *) "CameraInfo", (char *) "Pentax CameraInfo Tags"},
    {  0x0216, (char *) "BatteryInfo", (char *) "Pentax BatteryInfo Tags"},
    {  0x021B, (char *) "SaturationInfo", (char *) nullptr},
    {  0x021F, (char *) "AFInfo", (char *) "Pentax AFInfo Tags"},
    {  0x0222, (char *) "ColorInfo", (char *) "Pentax ColorInfo Tags"},
    {  0x0224, (char *) "EVStepInfo", (char *) "Pentax EVStepInfo Tags"},
	{  0x0226, (char *) "ShotInfo", (char *) "Pentax ShotInfo Tags"},
	{  0x0227, (char *) "FacePos", (char *) "Pentax FacePos Tags"},
	{  0x0228, (char *) "FaceSize", (char *) "Pentax FaceSize Tags"},
	{  0x0229, (char *) "SerialNumber", (char *) nullptr},
	{  0x022A, (char *) "FilterInfo", (char *) "Pentax FilterInfo Tags"},
	{  0x022B, (char *) "LevelInfo", (char *) "Pentax LevelInfo Tags"},
	{  0x022E, (char *) "Artist", (char *) nullptr},
	{  0x022F, (char *) "Copyright", (char *) nullptr},
	{  0x0230, (char *) "FirmwareVersion", (char *) nullptr},
	{  0x0231, (char *) "ContrastDetectAFArea", (char *) nullptr},
	{  0x0235, (char *) "CrossProcessParams", (char *) nullptr},
	{  0x03FE, (char *) "DataDump", (char *) nullptr},
    {  0x0402, (char *) "ToneCurve", (char *) nullptr},
    {  0x0403, (char *) "ToneCurves", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x1000, (char *) "HometownCityCode", (char *) nullptr},
    {  0x1001, (char *) "DestinationCityCode", (char *) nullptr},
    {  0x2000, (char *) "PreviewImageData", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Sony maker note
*/
static TagInfo
  exif_sony_tag_table[] =
  {
    {  0x0102, (char *) "Quality", (char *) nullptr},
    {  0x0104, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x0105, (char *) "Teleconverter", (char *) nullptr},
    {  0x0112, (char *) "WhiteBalanceFineTune", (char *) nullptr},
    {  0x0114, (char *) "CameraSettings", (char *) nullptr},
    {  0x0115, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0E00, (char *) "PrintIM", (char *) nullptr},
    {  0x1000, (char *) "MultiBurstMode", (char *) nullptr},
    {  0x1001, (char *) "MultiBurstImageWidth", (char *) nullptr},
    {  0x1002, (char *) "MultiBurstImageHeight", (char *) nullptr},
    {  0x1003, (char *) "Panorama", (char *) nullptr},
    {  0x2001, (char *) "PreviewImage", (char *) nullptr},
    {  0x2004, (char *) "Contrast", (char *) nullptr},
    {  0x2005, (char *) "Saturation", (char *) nullptr},
    {  0x2006, (char *) "Sharpness", (char *) nullptr},
    {  0x2007, (char *) "Brightness", (char *) nullptr},
    {  0x2008, (char *) "LongExposureNoiseReduction", (char *) nullptr},
    {  0x2009, (char *) "HighISONoiseReduction", (char *) nullptr},
    {  0x200A, (char *) "HDR", (char *) nullptr},
    {  0x200B, (char *) "MultiFrameNoiseReduction", (char *) nullptr},
    {  0x3000, (char *) "ShotInfo", (char *) nullptr},
    {  0xB000, (char *) "FileFormat", (char *) nullptr},
    {  0xB001, (char *) "SonyModelID", (char *) nullptr},
    {  0xB020, (char *) "ColorReproduction", (char *) nullptr},
    {  0xB021, (char *) "ColorTemperature", (char *) nullptr},
    {  0xB022, (char *) "ColorCompensationFilter", (char *) nullptr},
    {  0xB023, (char *) "SceneMode", (char *) nullptr},
    {  0xB024, (char *) "ZoneMatching", (char *) nullptr},
    {  0xB025, (char *) "DynamicRangeOptimizer", (char *) nullptr},
    {  0xB026, (char *) "ImageStabilization", (char *) nullptr},
    {  0xB027, (char *) "LensType", (char *) nullptr},
    {  0xB028, (char *) "MinoltaMakerNote", (char *) nullptr},
    {  0xB029, (char *) "ColorMode", (char *) nullptr},
    {  0xB02B, (char *) "FullImageSize", (char *) nullptr},
    {  0xB02C, (char *) "PreviewImageSize", (char *) nullptr},
    {  0xB040, (char *) "Macro", (char *) nullptr},
    {  0xB041, (char *) "ExposureMode", (char *) nullptr},
    {  0xB042, (char *) "FocusMode", (char *) nullptr},
    {  0xB043, (char *) "AFMode", (char *) nullptr},
    {  0xB044, (char *) "AFIlluminator", (char *) nullptr},
    {  0xB047, (char *) "Quality2", (char *) nullptr},
    {  0xB048, (char *) "FlashLevel", (char *) nullptr},
    {  0xB049, (char *) "ReleaseMode", (char *) nullptr},
    {  0xB04A, (char *) "SequenceNumber", (char *) nullptr},
    {  0xB04B, (char *) "Anti-Blur", (char *) nullptr},
    {  0xB04E, (char *) "LongExposureNoiseReduction", (char *) nullptr},
    {  0xB04F, (char *) "DynamicRangeOptimizer", (char *) nullptr},
    {  0xB052, (char *) "IntelligentAuto", (char *) nullptr},
    {  0xB054, (char *) "WhiteBalance2", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Sigma SD1 maker note
*/
static TagInfo
  exif_sigma_sd1_tag_table[] =
  {
    {  0x0002, (char *) "SerialNumber", (char *) nullptr},
    {  0x0003, (char *) "DriveMode", (char *) nullptr},
    {  0x0004, (char *) "ResolutionMode", (char *) nullptr},
    {  0x0005, (char *) "AFMode", (char *) nullptr},
    {  0x0006, (char *) "FocusSetting", (char *) nullptr},
    {  0x0007, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0008, (char *) "ExposureMode", (char *) nullptr},
    {  0x0009, (char *) "MeteringMode", (char *) nullptr},
    {  0x000A, (char *) "LensFocalRange", (char *) nullptr},
    {  0x000B, (char *) "ColorSpace", (char *) nullptr},
    {  0x000C, (char *) "ExposureCompensation", (char *) nullptr},
    {  0x000D, (char *) "Contrast", (char *) nullptr},
    {  0x000E, (char *) "Shadow", (char *) nullptr},
    {  0x000F, (char *) "Highlight", (char *) nullptr},
    {  0x0010, (char *) "Saturation", (char *) nullptr},
    {  0x0011, (char *) "Sharpness", (char *) nullptr},
    {  0x0012, (char *) "X3FillLight", (char *) nullptr},
    {  0x0014, (char *) "ColorAdjustment", (char *) nullptr},
    {  0x0015, (char *) "AdjustmentMode", (char *) nullptr},
    {  0x0016, (char *) "Quality", (char *) nullptr},
    {  0x0017, (char *) "Firmware", (char *) nullptr},
    {  0x0018, (char *) "Software", (char *) nullptr},
    {  0x0019, (char *) "AutoBracket", (char *) nullptr},
    {  0x001A, (char *) "ChrominanceNoiseReduction", (char *) nullptr},
    {  0x001B, (char *) "LuminanceNoiseReduction", (char *) nullptr},
    {  0x001C, (char *) "PreviewImageStart", (char *) nullptr},
    {  0x001D, (char *) "PreviewImageLength", (char *) nullptr},
    {  0x001F, (char *) "MakerNoteVersion", (char *) nullptr},
    {  0x0026, (char *) "FileFormat", (char *) nullptr},
    {  0x002C, (char *) "ColorMode", (char *) nullptr},
    {  0x0030, (char *) "Calibration", (char *) nullptr},
    {  0x0048, (char *) "LensApertureRange", (char *) nullptr},
    {  0x0049, (char *) "FNumber", (char *) nullptr},
    {  0x004A, (char *) "ExposureTime", (char *) nullptr},
    {  0x004B, (char *) "ExposureTime2", (char *) nullptr},
    {  0x004D, (char *) "ExposureCompensation_SD1", (char *) nullptr},
    {  0x0055, (char *) "SensorTemperature", (char *) nullptr},
    {  0x0056, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x0057, (char *) "Firmware_SD1", (char *) nullptr},
    {  0x0058, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

/**
Sigma / Foveon maker note (others than SD1 models)
NB: many tags are not consistent between different models
*/
static TagInfo
  exif_sigma_foveon_tag_table[] =
  {
    {  0x0002, (char *) "SerialNumber", (char *) nullptr},
    {  0x0003, (char *) "DriveMode", (char *) nullptr},
    {  0x0004, (char *) "ResolutionMode", (char *) nullptr},
    {  0x0005, (char *) "AFMode", (char *) nullptr},
    {  0x0006, (char *) "FocusSetting", (char *) nullptr},
    {  0x0007, (char *) "WhiteBalance", (char *) nullptr},
    {  0x0008, (char *) "ExposureMode", (char *) nullptr},
    {  0x0009, (char *) "MeteringMode", (char *) nullptr},
    {  0x000A, (char *) "LensFocalRange", (char *) nullptr},
    {  0x000B, (char *) "ColorSpace", (char *) nullptr},
    {  0x000C, (char *) "ExposureCompensation", (char *) nullptr},
    {  0x000D, (char *) "Contrast", (char *) nullptr},
    {  0x000E, (char *) "Shadow", (char *) nullptr},
    {  0x000F, (char *) "Highlight", (char *) nullptr},
    {  0x0010, (char *) "Saturation", (char *) nullptr},
    {  0x0011, (char *) "Sharpness", (char *) nullptr},
    {  0x0012, (char *) "X3FillLight", (char *) nullptr},
    {  0x0014, (char *) "ColorAdjustment", (char *) nullptr},
    {  0x0015, (char *) "AdjustmentMode", (char *) nullptr},
    {  0x0016, (char *) "Quality", (char *) nullptr},
    {  0x0017, (char *) "Firmware", (char *) nullptr},
    {  0x0018, (char *) "Software", (char *) nullptr},
    {  0x0019, (char *) "AutoBracket", (char *) nullptr},
    {  0x001A, (char *) "PreviewImageStart", (char *) nullptr},
    {  0x001B, (char *) "PreviewImageLength", (char *) nullptr},
    {  0x001C, (char *) "PreviewImageSize", (char *) nullptr},
    {  0x001D, (char *) "MakerNoteVersion", (char *) nullptr},
    {  0x001F, (char *) "AFPoint", (char *) nullptr},
    {  0x0022, (char *) "FileFormat", (char *) nullptr},
    {  0x0024, (char *) "Calibration", (char *) nullptr},
    {  0x002C, (char *) "ColorMode", (char *) nullptr},
    {  0x0030, (char *) "LensApertureRange", (char *) nullptr},
    {  0x0031, (char *) "FNumber", (char *) nullptr},
    {  0x0032, (char *) "ExposureTime", (char *) nullptr},
    {  0x0033, (char *) "ExposureTime2", (char *) nullptr},
    {  0x0034, (char *) "BurstShot", (char *) nullptr},
    {  0x0035, (char *) "ExposureCompensation", (char *) nullptr},
    {  0x0039, (char *) "SensorTemperature", (char *) nullptr},
    {  0x003A, (char *) "FlashExposureComp", (char *) nullptr},
    {  0x003B, (char *) "Firmware", (char *) nullptr},
    {  0x003C, (char *) "WhiteBalance", (char *) nullptr},
    {  0x003D, (char *) "PictureMode", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// IPTC tags definition
// --------------------------------------------------------------------------

static TagInfo
  iptc_tag_table[] =
  {
	  // IPTC-NAA IIM version 4
    {  0x0200 +   0, (char *) "ApplicationRecordVersion", (char *) "Application Record Version"},
    {  0x0200 +   3, (char *) "ObjectTypeReference", (char *) "Object Type Reference"},
    {  0x0200 +   4, (char *) "ObjectAttributeReference", (char *) "Object Attribute Reference"},
    {  0x0200 +   5, (char *) "ObjectName", (char *) "Title"},
    {  0x0200 +   7, (char *) "EditStatus", (char *) "Edit Status"},
    {  0x0200 +   8, (char *) "EditorialUpdate", (char *) "Editorial Update"},
    {  0x0200 +  10, (char *) "Urgency", (char *) "Urgency"},
    {  0x0200 +  12, (char *) "SubjectReference", (char *) "Subject Reference"},
    {  0x0200 +  15, (char *) "Category", (char *) "Category"},
    {  0x0200 +  20, (char *) "SupplementalCategories", (char *) "Supplemental Categories"},
    {  0x0200 +  22, (char *) "FixtureIdentifier", (char *) "Fixture Identifier"},
    {  0x0200 +  25, (char *) "Keywords", (char *) "Keywords"},
    {  0x0200 +  26, (char *) "ContentLocationCode", (char *) "Content Location Code"},
    {  0x0200 +  27, (char *) "ContentLocationName", (char *) "Content Location Name"},
    {  0x0200 +  30, (char *) "ReleaseDate", (char *) "Release Date"},
    {  0x0200 +  35, (char *) "ReleaseTime", (char *) "Release Time"},
    {  0x0200 +  37, (char *) "ExpirationDate", (char *) "Expiration Date"},
    {  0x0200 +  38, (char *) "ExpirationTime", (char *) "Expiration Time"},
    {  0x0200 +  40, (char *) "SpecialInstructions", (char *) "Instructions"},
    {  0x0200 +  42, (char *) "ActionAdvised", (char *) "Action Advised"},
    {  0x0200 +  45, (char *) "ReferenceService", (char *) "Reference Service"},
    {  0x0200 +  47, (char *) "ReferenceDate", (char *) "Reference Date"},
    {  0x0200 +  50, (char *) "ReferenceNumber", (char *) "Reference Number"},
    {  0x0200 +  55, (char *) "DateCreated", (char *) "Date Created"},
    {  0x0200 +  60, (char *) "TimeCreated", (char *) "Time Created"},
    {  0x0200 +  62, (char *) "DigitalCreationDate", (char *) "Digital Creation Date"},
    {  0x0200 +  63, (char *) "DigitalCreationTime", (char *) "Digital Creation Time"},
    {  0x0200 +  65, (char *) "OriginatingProgram", (char *) "Originating Program"},
    {  0x0200 +  70, (char *) "ProgramVersion", (char *) "Program Version"},
    {  0x0200 +  75, (char *) "ObjectCycle", (char *) "Object Cycle"},
    {  0x0200 +  80, (char *) "By-line", (char *) "Author"},
    {  0x0200 +  85, (char *) "By-lineTitle", (char *) "Author's Position"},
    {  0x0200 +  90, (char *) "City", (char *) "City"},
    {  0x0200 +  92, (char *) "SubLocation", (char *) "Sub-Location"},
    {  0x0200 +  95, (char *) "Province-State", (char *) "State/Province"},
    {  0x0200 + 100, (char *) "Country-PrimaryLocationCode", (char *) "Country Code"},
    {  0x0200 + 101, (char *) "Country-PrimaryLocationName", (char *) "Country Name"},
    {  0x0200 + 103, (char *) "OriginalTransmissionReference", (char *) "Transmission Reference"},
    {  0x0200 + 105, (char *) "Headline", (char *) "Headline"},
    {  0x0200 + 110, (char *) "Credit", (char *) "Credit"},
    {  0x0200 + 115, (char *) "Source", (char *) "Source"},
    {  0x0200 + 116, (char *) "CopyrightNotice", (char *) "Copyright Notice"},
    {  0x0200 + 118, (char *) "Contact", (char *) "Contact"},
    {  0x0200 + 120, (char *) "Caption-Abstract", (char *) "Caption"},
    {  0x0200 + 122, (char *) "Writer-Editor", (char *) "Caption Writer"},
    {  0x0200 + 125, (char *) "RasterizedCaption", (char *) "Rasterized Caption"},
    {  0x0200 + 130, (char *) "ImageType", (char *) "Image Type"},
    {  0x0200 + 131, (char *) "ImageOrientation", (char *) "Image Orientation"},
    {  0x0200 + 135, (char *) "LanguageIdentifier", (char *) "Language Identifier"},
    {  0x0200 + 150, (char *) "AudioType", (char *) "Audio Type"},
    {  0x0200 + 151, (char *) "AudioSamplingRate", (char *) "Audio Sampling Rate"},
    {  0x0200 + 152, (char *) "AudioSamplingResolution", (char *) "Audio Sampling Resolution"},
    {  0x0200 + 153, (char *) "AudioDuration", (char *) "Audio Duration"},
    {  0x0200 + 154, (char *) "AudioOutcue", (char *) "Audio Outcue"},
		// Metadata seen in other softwares (see also http://owl.phy.queensu.ca/~phil/exiftool/TagNames/IPTC.html#ApplicationRecord)
    {  0x0200 + 184, (char *) "JobID", (char *) "Job ID"},
    {  0x0200 + 185, (char *) "MasterDocumentID", (char *) "Master Document ID"},
    {  0x0200 + 186, (char *) "ShortDocumentID", (char *) "Short Document ID"},
    {  0x0200 + 187, (char *) "UniqueDocumentID", (char *) "Unique Document ID"},
    {  0x0200 + 188, (char *) "OwnerID", (char *) "Owner ID"},
		// IPTC-NAA IIM version 4
    {  0x0200 + 200, (char *) "ObjectPreviewFileFormat", (char *) "Object Preview File Format"},
    {  0x0200 + 201, (char *) "ObjectPreviewFileVersion", (char *) "Object Preview File Version"},
    {  0x0200 + 202, (char *) "ObjectPreviewData", (char *) "Audio Outcue"},
		// Metadata seen in other softwares (see also http://owl.phy.queensu.ca/~phil/exiftool/TagNames/IPTC.html#ApplicationRecord)
    {  0x0200 + 221, (char *) "Prefs", (char *) "PhotoMechanic preferences"},
    {  0x0200 + 225, (char *) "ClassifyState", (char *) "Classify State"},
    {  0x0200 + 228, (char *) "SimilarityIndex", (char *) "Similarity Index"},
    {  0x0200 + 230, (char *) "DocumentNotes", (char *) "Document Notes"},
    {  0x0200 + 231, (char *) "DocumentHistory", (char *) "Document History"},
    {  0x0200 + 232, (char *) "ExifCameraInfo", (char *) "Exif Camera Info"},

    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// GeoTIFF tags definition
// --------------------------------------------------------------------------

static TagInfo
  geotiff_tag_table[] =
  {
    {  0x830E, (char *) "GeoPixelScale", (char *) nullptr},
    {  0x8480, (char *) "Intergraph TransformationMatrix", (char *) nullptr},
    {  0x8482, (char *) "GeoTiePoints", (char *) nullptr},
    {  0x85D7, (char *) "JPL Carto IFD offset", (char *) nullptr},
    {  0x85D8, (char *) "GeoTransformationMatrix", (char *) nullptr},
    {  0x87AF, (char *) "GeoKeyDirectory", (char *) nullptr},
    {  0x87B0, (char *) "GeoDoubleParams", (char *) nullptr},
    {  0x87B1, (char *) "GeoASCIIParams", (char *) nullptr},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// Animation tags definition
// --------------------------------------------------------------------------

static TagInfo
  animation_tag_table[] =
  {
    {  0x0001, (char *) "LogicalWidth", (char *) "Logical width"},
    {  0x0002, (char *) "LogicalHeight", (char *) "Logical height"},
    {  0x0003, (char *) "GlobalPalette", (char *) "Global Palette"},
    {  0x0004, (char *) "Loop", (char *) "loop"},
    {  0x1001, (char *) "FrameLeft", (char *) "Frame left"},
    {  0x1002, (char *) "FrameTop", (char *) "Frame top"},
    {  0x1003, (char *) "NoLocalPalette", (char *) "No Local Palette"},
    {  0x1004, (char *) "Interlaced", (char *) "Interlaced"},
    {  0x1005, (char *) "FrameTime", (char *) "Frame display time"},
    {  0x1006, (char *) "DisposalMethod", (char *) "Frame disposal method"},
    {  0x0000, (char *) nullptr, (char *) nullptr}
  };

// --------------------------------------------------------------------------
// TagLib class definition
// --------------------------------------------------------------------------


/**
This is where the tag info tables are initialized
*/
TagLib::TagLib() {
	// initialize all known metadata models
	// ====================================

	// Exif
	addMetadataModel(TagLib::EXIF_MAIN, exif_exif_tag_table);
	addMetadataModel(TagLib::EXIF_EXIF, exif_exif_tag_table);
	addMetadataModel(TagLib::EXIF_GPS, exif_gps_tag_table);
	addMetadataModel(TagLib::EXIF_INTEROP, exif_interop_tag_table);

	// Exif maker note
	addMetadataModel(TagLib::EXIF_MAKERNOTE_CANON, exif_canon_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_CASIOTYPE1, exif_casio_type1_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_CASIOTYPE2, exif_casio_type2_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_FUJIFILM, exif_fujifilm_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_KYOCERA, exif_kyocera_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_MINOLTA, exif_minolta_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_NIKONTYPE1, exif_nikon_type1_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_NIKONTYPE2, exif_nikon_type2_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_NIKONTYPE3, exif_nikon_type3_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_OLYMPUSTYPE1, exif_olympus_type1_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_PANASONIC, exif_panasonic_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_ASAHI, exif_asahi_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_PENTAX, exif_pentax_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_SONY, exif_sony_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_SIGMA_SD1, exif_sigma_sd1_tag_table);
	addMetadataModel(TagLib::EXIF_MAKERNOTE_SIGMA_FOVEON, exif_sigma_foveon_tag_table);

	// IPTC/NAA
	addMetadataModel(TagLib::IPTC, iptc_tag_table);

	// GeoTIFF
	addMetadataModel(TagLib::GEOTIFF, geotiff_tag_table);

	// Animation
	addMetadataModel(TagLib::ANIMATION, animation_tag_table);
}

FIBOOL TagLib::addMetadataModel(MDMODEL md_model, TagInfo *tag_table) {
	// check that the model doesn't already exist
	if ((_table_map.find(md_model) == _table_map.end()) && tag_table) {

		// add the metadata model
		TAGINFO& info_map = _table_map[md_model];

		// add the tag description table
		for (int i = 0; tag_table[i].tag || tag_table[i].fieldname; ++i) {
			info_map[tag_table[i].tag] = &tag_table[i];
		}

		return TRUE;
	}

	return FALSE;
}

TagLib::~TagLib() = default;


const TagLib& 
TagLib::instance() {
	static TagLib s;
	return s;
}

const TagInfo* 
TagLib::getTagInfo(MDMODEL md_model, uint16_t tagID) const {

	if (auto it = _table_map.find(md_model); it != _table_map.end()) {

		const TAGINFO &info_map = it->second;
		if (auto i = info_map.find(tagID); i != info_map.end()) {
			return i->second;
		}
	}
	return nullptr;
}

const char* 
TagLib::getTagFieldName(MDMODEL md_model, uint16_t tagID, char *defaultKey) const {

	const TagInfo *info = getTagInfo(md_model, tagID);
	if (!info) {
		if (defaultKey) {
			snprintf(defaultKey, 16, "Tag 0x%04X", tagID);
			return defaultKey;
		} else {
			return nullptr;
		}
	}

	return info->fieldname;
}

const char* 
TagLib::getTagDescription(MDMODEL md_model, uint16_t tagID) const {

	const TagInfo *info = getTagInfo(md_model, tagID);
	if (info) {
		return info->description;
	}

	return nullptr;
}

int TagLib::getTagID(MDMODEL md_model, const char *key) const {

	if (auto it = _table_map.find(md_model); it != _table_map.end()) {

		const TAGINFO &info_map = it->second;
		for (auto i = info_map.begin(); i != info_map.end(); ++i) {
			const TagInfo *info = i->second;
			if (info && (strcmp(info->fieldname, key) == 0)) {
				return (int)info->tag;
			}
		}
	}
	return -1;
}

FREE_IMAGE_MDMODEL 
TagLib::getFreeImageModel(MDMODEL model) const {
	switch (model) {
		case EXIF_MAIN:
			return FIMD_EXIF_MAIN;

		case EXIF_EXIF:
			return FIMD_EXIF_EXIF;

		case EXIF_GPS: 
			return FIMD_EXIF_GPS;

		case EXIF_INTEROP:
			return FIMD_EXIF_INTEROP;

		case EXIF_MAKERNOTE_CANON:
		case EXIF_MAKERNOTE_CASIOTYPE1:
		case EXIF_MAKERNOTE_CASIOTYPE2:
		case EXIF_MAKERNOTE_FUJIFILM:
		case EXIF_MAKERNOTE_KYOCERA:
		case EXIF_MAKERNOTE_MINOLTA:
		case EXIF_MAKERNOTE_NIKONTYPE1:
		case EXIF_MAKERNOTE_NIKONTYPE2:
		case EXIF_MAKERNOTE_NIKONTYPE3:
		case EXIF_MAKERNOTE_OLYMPUSTYPE1:
		case EXIF_MAKERNOTE_PANASONIC:
		case EXIF_MAKERNOTE_ASAHI:
		case EXIF_MAKERNOTE_PENTAX:
		case EXIF_MAKERNOTE_SONY:
		case EXIF_MAKERNOTE_SIGMA_SD1:
		case EXIF_MAKERNOTE_SIGMA_FOVEON:
			return FIMD_EXIF_MAKERNOTE;

		case IPTC:
			return FIMD_IPTC;

		case GEOTIFF:
			return FIMD_GEOTIFF;

		case ANIMATION:
			return FIMD_ANIMATION;
	}

	return FIMD_NODATA;
}

