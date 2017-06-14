#ifndef MUPDF_FITZ_COLORSPACE_H
#define MUPDF_FITZ_COLORSPACE_H

#include "mupdf/fitz/system.h"
#include "mupdf/fitz/context.h"
#include "mupdf/fitz/store.h"

enum { FZ_MAX_COLORS = 32 };

enum
{
	/* Same order as needed by lcms */
	FZ_RI_PERCEPTUAL,
	FZ_RI_RELATIVECOLORIMETRIC,
	FZ_RI_SATURATION,
	FZ_RI_ABSOLUTECOLORIMETRIC,
};

typedef struct fz_color_params_s fz_color_params;

struct fz_color_params_s
{
	uint8_t ri;
	uint8_t bp;
	uint8_t op;
	uint8_t opm;
};


int fz_lookup_rendering_intent(const char *name);
char *fz_rendering_intent_name(int ri);

/*
	A fz_colorspace object represents an abstract colorspace. While
	this should be treated as a black box by callers of the library at
	this stage, know that it encapsulates knowledge of how to convert
	colors to and from the colorspace, any lookup tables generated, the
	number of components in the colorspace etc.
*/
typedef struct fz_colorspace_s fz_colorspace;

/*
	A fz_iccprofile object encapsulates details about the icc profile. It
	also includes the profile handle provided by the cmm and as such is used
	in the creation of links between color spaces.
*/
typedef struct fz_iccprofile_s fz_iccprofile;

/*
	A fz_icclink object encapsulates details about the link between profiles.
*/
typedef struct fz_icclink_s fz_icclink;

/*
	Used to communicate any document internal page specific default color spaces.
*/
typedef struct fz_page_default_cs_s fz_page_default_cs;

/*
	fz_colorspace_is_subtractive: Return true if a colorspace is subtractive.

	True for CMYK, Separation and DeviceN colorspaces.
*/
int fz_colorspace_is_subtractive(fz_context *ctx, fz_colorspace *pix);

/*
	fz_device_gray: Get colorspace representing device specific gray.
*/
fz_colorspace *fz_device_gray(fz_context *ctx);

/*
	fz_device_rgb: Get colorspace representing device specific rgb.
*/
fz_colorspace *fz_device_rgb(fz_context *ctx);

/*
	fz_device_bgr: Get colorspace representing device specific bgr.
*/
fz_colorspace *fz_device_bgr(fz_context *ctx);

/*
	fz_device_cmyk: Get colorspace representing device specific CMYK.
*/
fz_colorspace *fz_device_cmyk(fz_context *ctx);

/*
	fz_device_lab: Get colorspace representing device specific LAB.
*/
fz_colorspace *fz_device_lab(fz_context *ctx);

/*
	fz_cs_params: Get default color params for general color conversion.
*/
fz_color_params *fz_cs_params(fz_context *ctx);

typedef void (fz_colorspace_convert_fn)(fz_context *ctx, fz_colorspace *cs, const float *src, float *dst);

typedef void (fz_colorspace_destruct_fn)(fz_context *ctx, fz_colorspace *cs);

typedef fz_colorspace *(fz_colorspace_base_cs_fn)(const fz_colorspace *cs);

typedef void (fz_colorspace_clamp_fn)(const fz_colorspace *cs, const float *src, float *dst);

fz_colorspace *fz_new_colorspace(fz_context *ctx, char *name, int is_static, int n, int is_subtractive, fz_colorspace_convert_fn *to_rgb, fz_colorspace_convert_fn *from_rgb, fz_colorspace_base_cs_fn *base, fz_colorspace_clamp_fn *clamp, fz_colorspace_destruct_fn *destruct, void *data, size_t size);
fz_colorspace *fz_new_indexed_colorspace(fz_context *ctx, fz_colorspace *base, int high, unsigned char *lookup);
fz_colorspace *fz_keep_colorspace(fz_context *ctx, fz_colorspace *colorspace);
void fz_drop_colorspace(fz_context *ctx, fz_colorspace *colorspace);
void fz_drop_colorspace_imp(fz_context *ctx, fz_storable *colorspace);

fz_colorspace *fz_colorspace_base(const fz_colorspace *cs);
int fz_colorspace_is_icc(const fz_colorspace *cs);
int fz_colorspace_is_indexed(const fz_colorspace *cs);
int fz_colorspace_n(fz_context *ctx, const fz_colorspace *cs);
const char *fz_colorspace_name(fz_context *ctx, const fz_colorspace *cs);
void fz_clamp_color(fz_context *ctx, const fz_colorspace *cs, const float *in, float *out);
void fz_convert_color(fz_context *ctx, const fz_color_params *params, fz_colorspace *intcs, fz_colorspace *dscs, float *dstv, fz_colorspace *srcs, const float *srcv);

typedef struct fz_color_converter_s fz_color_converter;

/* This structure is public because it allows us to avoid dynamic allocations.
 * Callers should only rely on the convert entry - the rest of the structure
 * is subject to change without notice.
 */
struct fz_color_converter_s
{
	void (*convert)(fz_context *, fz_color_converter *, float *, const float *);
	fz_colorspace *ds;
	fz_colorspace *ss;
	fz_colorspace *is;
	void *opaque;
	void *link;
	int n;
};

void fz_lookup_color_converter(fz_context *ctx, fz_color_converter *cc, fz_colorspace *is, fz_colorspace *ds, fz_colorspace *ss, const fz_color_params *params);
void fz_discard_color_converter(fz_context *ctx, fz_color_converter *cc);
void fz_init_cached_color_converter(fz_context *ctx, fz_color_converter *cc, fz_colorspace *is, fz_colorspace *ds, fz_colorspace *ss, const fz_color_params *params);
void fz_fin_cached_color_converter(fz_context *ctx, fz_color_converter *cc);

/* Public to allow use in icc creation */
typedef struct fz_cal_color_s fz_cal_color;

struct fz_cal_color_s {
	float wp[3];
	float bp[3];
	float gamma[3];
	float matrix[9];
	int n;
	fz_iccprofile *profile;
};

/*
	icc methods
*/
fz_colorspace *fz_new_icc_colorspace(fz_context *ctx, int is_static, int num, fz_buffer *buf, const char *name);
fz_colorspace *fz_new_cal_colorspace(fz_context *ctx, float *wp, float *bp, float *gamma, float *matrix);
int fz_create_icc_from_cal(fz_context *ctx, unsigned char **buffer, fz_cal_color *cal);
unsigned char *fz_get_icc_data(fz_context *ctx, fz_colorspace *cs, int *size);

void fz_color_param_init(fz_color_params *color_params);

/* Default cs */
fz_page_default_cs *fz_new_default_cs(fz_context *ctx);
fz_page_default_cs* fz_keep_default_cs(fz_context *ctx, fz_page_default_cs *default_cs);
void fz_drop_default_cs(fz_context *ctx, fz_page_default_cs *default_cs);

/* Do we want to make fz_page_default_cs public and get rid of these? */
void fz_set_default_gray(fz_context *ctx, fz_page_default_cs *default_cs, fz_colorspace *cs);
void fz_set_default_rgb(fz_context *ctx, fz_page_default_cs *default_cs, fz_colorspace *cs);
void fz_set_default_cmyk(fz_context *ctx, fz_page_default_cs *default_cs, fz_colorspace *cs);
void fz_set_default_oi(fz_context *ctx, fz_page_default_cs *default_cs, fz_colorspace *cs);

fz_colorspace *fz_get_default_gray(fz_context *ctx, fz_page_default_cs *default_cs);
fz_colorspace *fz_get_default_rgb(fz_context *ctx, fz_page_default_cs *default_cs);
fz_colorspace *fz_get_default_cmyk(fz_context *ctx, fz_page_default_cs *default_cs);
fz_colorspace *fz_get_outputintent(fz_context *ctx, fz_page_default_cs *default_cs);

#endif
