// ==========================================================
// Tag to string conversion functions
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

#ifdef _MSC_VER 
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters
#endif

#include "FreeImage.h"
#include "Utilities.h"
#include "FreeImageTag.h"
#include "FIRational.h"

#define MAX_TEXT_EXTENT	512

/**
Convert a tag to a C string
*/
static const char* 
ConvertAnyTag(FITAG *tag) {
	char format[MAX_TEXT_EXTENT];
	static std::string buffer;
	uint32_t i;

	if (!tag)
		return nullptr;

	buffer.erase();
	
	// convert the tag value to a string buffer

	const FREE_IMAGE_MDTYPE tag_type = FreeImage_GetTagType(tag);
	uint32_t tag_count = FreeImage_GetTagCount(tag);

	switch (tag_type) {
		case FIDT_BYTE:		// N x 8-bit unsigned integer 
		{
			auto *pvalue = (const uint8_t*)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%d",	(int32_t) pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %d",	(int32_t) pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_SHORT:	// N x 16-bit unsigned integer 
		{
			auto *pvalue = (const unsigned short *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%hu", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %hu",	pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_LONG:		// N x 32-bit unsigned integer 
		{
			auto *pvalue = (const uint32_t *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%u", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %u",	pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_RATIONAL: // N x 64-bit unsigned fraction 
		{
			auto *pvalue = (const uint32_t*)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%u/%u", pvalue[0], pvalue[1]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %u/%u", pvalue[2*i], pvalue[2*i+1]);
				buffer += format;
			}
			break;
		}
		case FIDT_SBYTE:	// N x 8-bit signed integer 
		{
			auto *pvalue = (const char*)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%d",	(int32_t) pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %d",	(int32_t) pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_SSHORT:	// N x 16-bit signed integer 
		{
			auto *pvalue = (const short *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%hd", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %hd",	pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_SLONG:	// N x 32-bit signed integer 
		{
			auto *pvalue = (const int32_t *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%d", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %d",	pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_SRATIONAL:// N x 64-bit signed fraction 
		{
			auto *pvalue = (const int32_t*)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%d/%d", pvalue[0], pvalue[1]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %d/%d", pvalue[2*i], pvalue[2*i+1]);
				buffer += format;
			}
			break;
		}
		case FIDT_FLOAT:	// N x 32-bit IEEE floating point 
		{
			auto *pvalue = (const float *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%f", (double) pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), "%f", (double) pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_DOUBLE:	// N x 64-bit IEEE floating point 
		{
			auto *pvalue = (const double *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%lf", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), "%lf", pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_IFD:		// N x 32-bit unsigned integer (offset) 
		{
			auto *pvalue = (const uint32_t *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%X", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " %X",	pvalue[i]);
				buffer += format;
			}
			break;
		}
		case FIDT_PALETTE:	// N x 32-bit FIRGBA8 
		{
			auto *pvalue = (const FIRGBA8 *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "(%d,%d,%d,%d)", pvalue[0].red, pvalue[0].green, pvalue[0].blue, pvalue[0].alpha);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), " (%d,%d,%d,%d)", pvalue[i].red, pvalue[i].green, pvalue[i].blue, pvalue[i].alpha);
				buffer += format;
			}
			break;
		}
		
		case FIDT_LONG8:	// N x 64-bit unsigned integer 
		{
			auto *pvalue = (const uint64_t *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%llu", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), "%llu", pvalue[i]);
				buffer += format;
			}
			break;
		}

		case FIDT_IFD8:		// N x 64-bit unsigned integer (offset)
		{
			auto *pvalue = (const uint64_t *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%llX", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), "%llX", pvalue[i]);
				buffer += format;
			}
			break;
		}

		case FIDT_SLONG8:	// N x 64-bit signed integer
		{
			auto *pvalue = (const int64_t *)FreeImage_GetTagValue(tag);

			snprintf(format, std::size(format), "%lld", pvalue[0]);
			buffer += format;
			for (i = 1; i < tag_count; i++) {
				snprintf(format, std::size(format), "%lld", pvalue[i]);
				buffer += format;
			}
			break;
		}

		case FIDT_ASCII:	// 8-bit bytes w/ last byte null 
		case FIDT_UNDEFINED:// 8-bit untyped data 
		default:
		{
			int max_size = MIN((int)FreeImage_GetTagLength(tag), (int)MAX_TEXT_EXTENT);
			if (max_size == MAX_TEXT_EXTENT)
				max_size--;
			memcpy(format, (char*)FreeImage_GetTagValue(tag), max_size);
			format[max_size] = '\0';
			buffer += format;
			break;
		}
	}

	return buffer.c_str();
}

/**
Convert a Exif tag to a C string
*/
static const char* 
ConvertExifTag(FITAG *tag) {
	char format[MAX_TEXT_EXTENT];
	static std::string buffer;

	if (!tag)
		return nullptr;

	buffer.erase();

	// convert the tag value to a string buffer

	switch (FreeImage_GetTagID(tag)) {
		case TAG_ORIENTATION:
		{
			unsigned short orientation = *((unsigned short *)FreeImage_GetTagValue(tag));
			switch (orientation) {
				case 1:
					return "top, left side";
				case 2:
					return "top, right side";
				case 3:
					return "bottom, right side";
				case 4:
					return "bottom, left side";
				case 5:
					return "left side, top";
				case 6:
					return "right side, top";
				case 7:
					return "right side, bottom";
				case 8:
					return "left side, bottom";
				default:
					break;
			}
		}
		break;

		case TAG_REFERENCE_BLACK_WHITE:
		{
			auto *pvalue = (const uint32_t*)FreeImage_GetTagValue(tag);
			if (FreeImage_GetTagLength(tag) == 48) {
				// reference black point value and reference white point value (ReferenceBlackWhite)
				int blackR = 0, whiteR = 0, blackG = 0, whiteG = 0, blackB = 0, whiteB = 0;
				if (pvalue[1])
					blackR = (int)(pvalue[0] / pvalue[1]);
				if (pvalue[3])
					whiteR = (int)(pvalue[2] / pvalue[3]);
				if (pvalue[5])
					blackG = (int)(pvalue[4] / pvalue[5]);
				if (pvalue[7])
					whiteG = (int)(pvalue[6] / pvalue[7]);
				if (pvalue[9])
					blackB = (int)(pvalue[8] / pvalue[9]);
				if (pvalue[11])
					whiteB = (int)(pvalue[10] / pvalue[11]);

				snprintf(format, std::size(format), "[%d,%d,%d] [%d,%d,%d]", blackR, blackG, blackB, whiteR, whiteG, whiteB);
				buffer += format;
				return buffer.c_str();
			}

		}
		break;

		case TAG_COLOR_SPACE:
		{
			const auto colorSpace = *((unsigned short *)FreeImage_GetTagValue(tag));
			if (colorSpace == 1) {
				return "sRGB";
			} else if (colorSpace == 65535) {
				return "Undefined";
			} else {
				return "Unknown";
			}
		}
		break;

		case TAG_COMPONENTS_CONFIGURATION:
		{
			const char *componentStrings[7] = {"", "Y", "Cb", "Cr", "R", "G", "B"};
			auto *pvalue = (const uint8_t*)FreeImage_GetTagValue(tag);
			for (uint32_t i = 0; i < MIN((uint32_t)4, FreeImage_GetTagCount(tag)); i++) {
				int j = pvalue[i];
				if (j > 0 && j < 7)
					buffer += componentStrings[j];
			}
			return buffer.c_str();
		}
		break;

		case TAG_COMPRESSED_BITS_PER_PIXEL:
		{
			FIRational r(tag);
			buffer = r.toString();
			if (buffer == "1")
				buffer += " bit/pixel";
			else 
				buffer += " bits/pixel";
			return buffer.c_str();
		}
		break;

		case TAG_X_RESOLUTION:
		case TAG_Y_RESOLUTION:
		case TAG_FOCAL_PLANE_X_RES:
		case TAG_FOCAL_PLANE_Y_RES:
		case TAG_BRIGHTNESS_VALUE:
		case TAG_EXPOSURE_BIAS_VALUE:
		{
			FIRational r(tag);
			buffer = r.toString();
			return buffer.c_str();
		}
		break;

		case TAG_RESOLUTION_UNIT:
		case TAG_FOCAL_PLANE_UNIT:
		{
			const auto resolutionUnit = *((unsigned short *)FreeImage_GetTagValue(tag));
			switch (resolutionUnit) {
				case 1:
					return "(No unit)";
				case 2:
					return "inches";
				case 3:
					return "cm";
				default:
					break;
			}
		}
		break;

		case TAG_YCBCR_POSITIONING:
		{
			const auto yCbCrPosition = *((unsigned short *)FreeImage_GetTagValue(tag));
			switch (yCbCrPosition) {
				case 1:
					return "Center of pixel array";
				case 2:
					return "Datum point";
				default:
					break;
			}
		} 
		break;

		case TAG_EXPOSURE_TIME:
		{
			FIRational r(tag);
			buffer = r.toString();
			buffer += " sec";
			return buffer.c_str();
		}
		break;

		case TAG_SHUTTER_SPEED_VALUE:
		{
			FIRational r(tag);
			int32_t apexValue = r.longValue();
			int32_t apexPower = 1 << apexValue;
			snprintf(format, std::size(format), "1/%d sec", (int)apexPower);
			buffer += format;
			return buffer.c_str();
		}
		break;

		case TAG_APERTURE_VALUE:
		case TAG_MAX_APERTURE_VALUE:
		{
			FIRational r(tag);
			double apertureApex = r.doubleValue();
	        double rootTwo = sqrt((double)2);
			double fStop = pow(rootTwo, apertureApex);
			snprintf(format, std::size(format), "F%.1f", fStop);
			buffer += format;
			return buffer.c_str();
		}
		break;

		case TAG_FNUMBER:
		{
			FIRational r(tag);
			double fnumber = r.doubleValue();
			snprintf(format, std::size(format), "F%.1f", fnumber);
			buffer += format;
			return buffer.c_str();
		}
		break;

		case TAG_FOCAL_LENGTH:
		{
			FIRational r(tag);
			double focalLength = r.doubleValue();
			snprintf(format, std::size(format), "%.1f mm", focalLength);
			buffer += format;
			return buffer.c_str();
		}
		break;

		case TAG_FOCAL_LENGTH_IN_35MM_FILM:
		{
			const auto focalLength = *((unsigned short *)FreeImage_GetTagValue(tag));
			snprintf(format, std::size(format), "%hu mm", focalLength);
			buffer += format;
			return buffer.c_str();
		}
		break;

		case TAG_FLASH:
		{
			const auto flash = *((unsigned short *)FreeImage_GetTagValue(tag));
			switch (flash) {
				case 0x0000:
					return "Flash did not fire";
				case 0x0001:
					return "Flash fired";
				case 0x0005:
					return "Strobe return light not detected";
				case 0x0007:
					return "Strobe return light detected";
				case 0x0009:
					return "Flash fired, compulsory flash mode";
				case 0x000D:
					return "Flash fired, compulsory flash mode, return light not detected";
				case 0x000F:
					return "Flash fired, compulsory flash mode, return light detected";
				case 0x0010:
					return "Flash did not fire, compulsory flash mode";
				case 0x0018:
					return "Flash did not fire, auto mode";
				case 0x0019:
					return "Flash fired, auto mode";
				case 0x001D:
					return "Flash fired, auto mode, return light not detected";
				case 0x001F:
					return "Flash fired, auto mode, return light detected";
				case 0x0020:
					return "No flash function";
				case 0x0041:
					return "Flash fired, red-eye reduction mode";
				case 0x0045:
					return "Flash fired, red-eye reduction mode, return light not detected";
				case 0x0047:
					return "Flash fired, red-eye reduction mode, return light detected";
				case 0x0049:
					return "Flash fired, compulsory flash mode, red-eye reduction mode";
				case 0x004D:
					return "Flash fired, compulsory flash mode, red-eye reduction mode, return light not detected";
				case 0x004F:
					return "Flash fired, compulsory flash mode, red-eye reduction mode, return light detected";
				case 0x0059:
					return "Flash fired, auto mode, red-eye reduction mode";
				case 0x005D:
					return "Flash fired, auto mode, return light not detected, red-eye reduction mode";
				case 0x005F:
					return "Flash fired, auto mode, return light detected, red-eye reduction mode";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", flash);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_SCENE_TYPE:
		{
			const auto sceneType = *((uint8_t*)FreeImage_GetTagValue(tag));
			if (sceneType == 1) {
				return "Directly photographed image";
			} else {
				snprintf(format, std::size(format), "Unknown (%d)", sceneType);
				buffer += format;
				return buffer.c_str();
			}
		}
		break;

		case TAG_SUBJECT_DISTANCE:
		{
			FIRational r(tag);
			if (r.getNumerator() == 0xFFFFFFFF) {
				return "Infinity";
			} else if (r.getNumerator() == 0) {
				return "Distance unknown";
			} else {
				double distance = r.doubleValue();
				snprintf(format, std::size(format), "%.3f meters", distance);
				buffer += format;
				return buffer.c_str();
			}
		}
		break;
			
		case TAG_METERING_MODE:
		{
			const auto meteringMode = *((unsigned short *)FreeImage_GetTagValue(tag));
			switch (meteringMode) {
				case 0:
					return "Unknown";
				case 1:
					return "Average";
				case 2:
					return "Center weighted average";
				case 3:
					return "Spot";
				case 4:
					return "Multi-spot";
				case 5:
					return "Multi-segment";
				case 6:
					return "Partial";
				case 255:
					return "(Other)";
				default:
					return "";
			}
		}
		break;

		case TAG_LIGHT_SOURCE:
		{
			const auto lightSource = *((unsigned short *)FreeImage_GetTagValue(tag));
			switch (lightSource) {
				case 0:
					return "Unknown";
				case 1:
					return "Daylight";
				case 2:
					return "Fluorescent";
				case 3:
					return "Tungsten (incandescent light)";
				case 4:
					return "Flash";
				case 9:
					return "Fine weather";
				case 10:
					return "Cloudy weather";
				case 11:
					return "Shade";
				case 12:
					return "Daylight fluorescent (D 5700 - 7100K)";
				case 13:
					return "Day white fluorescent (N 4600 - 5400K)";
				case 14:
					return "Cool white fluorescent (W 3900 - 4500K)";
				case 15:
					return "White fluorescent (WW 3200 - 3700K)";
				case 17:
					return "Standard light A";
				case 18:
					return "Standard light B";
				case 19:
					return "Standard light C";
				case 20:
					return "D55";
				case 21:
					return "D65";
				case 22:
					return "D75";
				case 23:
					return "D50";
				case 24:
					return "ISO studio tungsten";
				case 255:
					return "(Other)";
				default:
					return "";
			}
		}
		break;

		case TAG_SENSING_METHOD:
		{
			const auto sensingMethod = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (sensingMethod) {
				case 1:
					return "(Not defined)";
				case 2:
					return "One-chip color area sensor";
				case 3:
					return "Two-chip color area sensor";
				case 4:
					return "Three-chip color area sensor";
				case 5:
					return "Color sequential area sensor";
				case 7:
					return "Trilinear sensor";
				case 8:
					return "Color sequential linear sensor";
				default:
					return "";
			}
		}
		break;

		case TAG_FILE_SOURCE:
		{
			const auto fileSource = *((uint8_t*)FreeImage_GetTagValue(tag));
			if (fileSource == 3) {
				return "Digital Still Camera (DSC)";
			} else {
				snprintf(format, std::size(format), "Unknown (%d)", fileSource);
				buffer += format;
				return buffer.c_str();
			}
        }
		break;

		case TAG_EXPOSURE_PROGRAM:
		{
			const auto exposureProgram = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (exposureProgram) {
				case 1:
					return "Manual control";
				case 2:
					return "Program normal";
				case 3:
					return "Aperture priority";
				case 4:
					return "Shutter priority";
				case 5:
					return "Program creative (slow program)";
				case 6:
					return "Program action (high-speed program)";
				case 7:
					return "Portrait mode";
				case 8:
					return "Landscape mode";
				default:
					snprintf(format, std::size(format), "Unknown program (%d)", exposureProgram);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_CUSTOM_RENDERED:
		{
			const auto customRendered = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (customRendered) {
				case 0:
					return "Normal process";
				case 1:
					return "Custom process";
				default:
					snprintf(format, std::size(format), "Unknown rendering (%d)", customRendered);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_EXPOSURE_MODE:
		{
			const auto exposureMode = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (exposureMode) {
				case 0:
					return "Auto exposure";
				case 1:
					return "Manual exposure";
				case 2:
					return "Auto bracket";
				default:
					snprintf(format, std::size(format), "Unknown mode (%d)", exposureMode);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_WHITE_BALANCE:
		{
			const auto whiteBalance = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (whiteBalance) {
				case 0:
					return "Auto white balance";
				case 1:
					return "Manual white balance";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", whiteBalance);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_SCENE_CAPTURE_TYPE:
		{
			const auto sceneType = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (sceneType) {
				case 0:
					return "Standard";
				case 1:
					return "Landscape";
				case 2:
					return "Portrait";
				case 3:
					return "Night scene";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", sceneType);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_GAIN_CONTROL:
		{
			const auto gainControl = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (gainControl) {
				case 0:
					return "None";
				case 1:
					return "Low gain up";
				case 2:
					return "High gain up";
				case 3:
					return "Low gain down";
				case 4:
					return "High gain down";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", gainControl);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_CONTRAST:
		{
			const auto contrast = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (contrast) {
				case 0:
					return "Normal";
				case 1:
					return "Soft";
				case 2:
					return "Hard";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", contrast);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_SATURATION:
		{
			const auto saturation = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (saturation) {
				case 0:
					return "Normal";
				case 1:
					return "Low saturation";
				case 2:
					return "High saturation";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", saturation);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_SHARPNESS:
		{
			const auto sharpness = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (sharpness) {
				case 0:
					return "Normal";
				case 1:
					return "Soft";
				case 2:
					return "Hard";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", sharpness);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_SUBJECT_DISTANCE_RANGE:
		{
			const auto distanceRange = *((unsigned short *)FreeImage_GetTagValue(tag));

			switch (distanceRange) {
				case 0:
					return "unknown";
				case 1:
					return "Macro";
				case 2:
					return "Close view";
				case 3:
					return "Distant view";
				default:
					snprintf(format, std::size(format), "Unknown (%d)", distanceRange);
					buffer += format;
					return buffer.c_str();
			}
		}
		break;

		case TAG_ISO_SPEED_RATINGS:
		{
			unsigned short isoEquiv = *((unsigned short *)FreeImage_GetTagValue(tag));
			if (isoEquiv < 50) {
				isoEquiv *= 200;
			}
			snprintf(format, std::size(format), "%d", isoEquiv);
			buffer += format;
			return buffer.c_str();
		}
		break;

		case TAG_USER_COMMENT:
		{
			// first 8 bytes are used to define an ID code
			// we assume this is an ASCII string
			auto *userComment = (const uint8_t*)FreeImage_GetTagValue(tag);
			for (uint32_t i = 8; i < FreeImage_GetTagLength(tag); i++) {
				buffer += userComment[i];
			}
			buffer += '\0';
			return buffer.c_str();
		}
		break;

		case TAG_COMPRESSION:
		{
			const auto compression = *((uint16_t*)FreeImage_GetTagValue(tag));
			switch (compression) {
				case TAG_COMPRESSION_NONE:
					snprintf(format, std::size(format), "dump mode (%d)", compression);
					break;
				case TAG_COMPRESSION_CCITTRLE:
					snprintf(format, std::size(format), "CCITT modified Huffman RLE (%d)", compression);
					break;
				case TAG_COMPRESSION_CCITTFAX3:
					snprintf(format, std::size(format), "CCITT Group 3 fax encoding (%d)", compression);
					break;
				/*
				case TAG_COMPRESSION_CCITT_T4:
					snprintf(format, std::size(format), "CCITT T.4 (TIFF 6 name) (%d)", compression);
					break;
				*/
				case TAG_COMPRESSION_CCITTFAX4:
					snprintf(format, std::size(format), "CCITT Group 4 fax encoding (%d)", compression);
					break;
				/*
				case TAG_COMPRESSION_CCITT_T6:
					snprintf(format, std::size(format), "CCITT T.6 (TIFF 6 name) (%d)", compression);
					break;
				*/
				case TAG_COMPRESSION_LZW:
					snprintf(format, std::size(format), "LZW (%d)", compression);
					break;
				case TAG_COMPRESSION_OJPEG:
					snprintf(format, std::size(format), "!6.0 JPEG (%d)", compression);
					break;
				case TAG_COMPRESSION_JPEG:
					snprintf(format, std::size(format), "JPEG (%d)", compression);
					break;
				case TAG_COMPRESSION_NEXT:
					snprintf(format, std::size(format), "NeXT 2-bit RLE (%d)", compression);
					break;
				case TAG_COMPRESSION_CCITTRLEW:
					snprintf(format, std::size(format), "CCITTRLEW (%d)", compression);
					break;
				case TAG_COMPRESSION_PACKBITS:
					snprintf(format, std::size(format), "PackBits Macintosh RLE (%d)", compression);
					break;
				case TAG_COMPRESSION_THUNDERSCAN:
					snprintf(format, std::size(format), "ThunderScan RLE (%d)", compression);
					break;
				case TAG_COMPRESSION_PIXARFILM:
					snprintf(format, std::size(format), "Pixar companded 10bit LZW (%d)", compression);
					break;
				case TAG_COMPRESSION_PIXARLOG:
					snprintf(format, std::size(format), "Pixar companded 11bit ZIP (%d)", compression);
					break;
				case TAG_COMPRESSION_DEFLATE:
					snprintf(format, std::size(format), "Deflate compression (%d)", compression);
					break;
				case TAG_COMPRESSION_ADOBE_DEFLATE:
					snprintf(format, std::size(format), "Adobe Deflate compression (%d)", compression);
					break;
				case TAG_COMPRESSION_DCS:
					snprintf(format, std::size(format), "Kodak DCS encoding (%d)", compression);
					break;
				case TAG_COMPRESSION_JBIG:
					snprintf(format, std::size(format), "ISO JBIG (%d)", compression);
					break;
				case TAG_COMPRESSION_SGILOG:
					snprintf(format, std::size(format), "SGI Log Luminance RLE (%d)", compression);
					break;
				case TAG_COMPRESSION_SGILOG24:
					snprintf(format, std::size(format), "SGI Log 24-bit packed (%d)", compression);
					break;
				case TAG_COMPRESSION_JP2000:
					snprintf(format, std::size(format), "Leadtools JPEG2000 (%d)", compression);
					break;
				case TAG_COMPRESSION_LZMA:
					snprintf(format, std::size(format), "LZMA2 (%d)", compression);
					break;
				default:
					snprintf(format, std::size(format), "Unknown type (%d)", compression);
					break;
			}

			buffer += format;
			return buffer.c_str();
		}
		break;
	}

	return ConvertAnyTag(tag);
}

/**
Convert a Exif GPS tag to a C string
*/
static const char* 
ConvertExifGPSTag(FITAG *tag) {
	char format[MAX_TEXT_EXTENT];
	static std::string buffer;

	if (!tag)
		return nullptr;

	buffer.erase();

	// convert the tag value to a string buffer

	switch (FreeImage_GetTagID(tag)) {
		case TAG_GPS_LATITUDE:
		case TAG_GPS_LONGITUDE:
		case TAG_GPS_TIME_STAMP:
		{
			auto *pvalue = (const uint32_t*)FreeImage_GetTagValue(tag);
			if (FreeImage_GetTagLength(tag) == 24) {
				// dd:mm:ss or hh:mm:ss
				int dd = 0, mm = 0;
				double ss = 0;

				// convert to seconds
				if (pvalue[1])
					ss += ((double)pvalue[0] / (double)pvalue[1]) * 3600;
				if (pvalue[3])
					ss += ((double)pvalue[2] / (double)pvalue[3]) * 60;
				if (pvalue[5])
					ss += ((double)pvalue[4] / (double)pvalue[5]);
				
				// convert to dd:mm:ss.ss
				dd = (int)(ss / 3600);
				mm = (int)(ss / 60) - dd * 60;
				ss = ss - dd * 3600 - mm * 60;

				snprintf(format, std::size(format), "%d:%d:%.2f", dd, mm, ss);
				buffer += format;
				return buffer.c_str();
			}
		}
		break;

		case TAG_GPS_VERSION_ID:
		case TAG_GPS_LATITUDE_REF:
		case TAG_GPS_LONGITUDE_REF:
		case TAG_GPS_ALTITUDE_REF:
		case TAG_GPS_ALTITUDE:
		case TAG_GPS_SATELLITES:
		case TAG_GPS_STATUS:
		case TAG_GPS_MEASURE_MODE:
		case TAG_GPS_DOP:
		case TAG_GPS_SPEED_REF:
		case TAG_GPS_SPEED:
		case TAG_GPS_TRACK_REF:
		case TAG_GPS_TRACK:
		case TAG_GPS_IMG_DIRECTION_REF:
		case TAG_GPS_IMG_DIRECTION:
		case TAG_GPS_MAP_DATUM:
		case TAG_GPS_DEST_LATITUDE_REF:
		case TAG_GPS_DEST_LATITUDE:
		case TAG_GPS_DEST_LONGITUDE_REF:
		case TAG_GPS_DEST_LONGITUDE:
		case TAG_GPS_DEST_BEARING_REF:
		case TAG_GPS_DEST_BEARING:
		case TAG_GPS_DEST_DISTANCE_REF:
		case TAG_GPS_DEST_DISTANCE:
		case TAG_GPS_PROCESSING_METHOD:
		case TAG_GPS_AREA_INFORMATION:
		case TAG_GPS_DATE_STAMP:
		case TAG_GPS_DIFFERENTIAL:
			break;
	}

	return ConvertAnyTag(tag);
}

// ==========================================================
// Tag to string conversion function
//

const char* DLL_CALLCONV 
FreeImage_TagToString(FREE_IMAGE_MDMODEL model, FITAG *tag, char *Make) {
	switch (model) {
		case FIMD_EXIF_MAIN:
		case FIMD_EXIF_EXIF:
			return ConvertExifTag(tag);

		case FIMD_EXIF_GPS:
			return ConvertExifGPSTag(tag);

		case FIMD_EXIF_MAKERNOTE:
			// We should use the Make string to select an appropriate conversion function
			// TO DO ...
			break;

		case FIMD_EXIF_INTEROP:
		default:
			break;
	}

	return ConvertAnyTag(tag);
}

