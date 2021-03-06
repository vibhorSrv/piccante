/*

PICCANTE
The hottest HDR imaging library!
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#ifndef PIC_FILTERING_FILTER_NON_LOCAL_MEANS_F_HPP
#define PIC_FILTERING_FILTER_NON_LOCAL_MEANS_F_HPP

#include <random>

#include "../filtering/filter.hpp"
#include "../features_matching/patch_comp.hpp"
#include "../filtering/filter_bilateral_2ds.hpp"
#include "../util/array.hpp"
#include "../point_samplers/sampler_random_m.hpp"

namespace pic {

/**
 * @brief The FilterNonLocalMeansF class
 */
class FilterNonLocalMeansF: public Filter
{
protected:
    float sigma_r, sigma_r_sq_2;
    int kernelSize, searchWindow, halfSearchWindow;
    float kernelSize_sq, h, h_sq;

    MRSamplers<2> *ms;
    int seed;

    /**
     * @brief ProcessBBox
     * @param dst
     * @param src
     * @param box
     */
    void ProcessBBox(Image *dst, ImageVec src, BBox *box);

public:

    /**
     * @brief FilterNonLocalMeansF
     */
    FilterNonLocalMeansF() : Filter()
    {
        seed = 1;
        ms = NULL;
    }

    /**
     * @brief FilterNonLocalMeansF
     * @param sigma_s
     * @param sigma_r
     */
    FilterNonLocalMeansF(int searchWindow, int kernelSize, float sigma_r);

    /**
     * @brief update
     * @param sigma_r
     */
    void update(int searchWindow, int kernelSize, float sigma_r);

};

PIC_INLINE FilterNonLocalMeansF::FilterNonLocalMeansF(int searchWindow, int kernelSize, float sigma_r) : Filter()
{
    update(searchWindow, kernelSize, sigma_r);
}

PIC_INLINE void FilterNonLocalMeansF::update(int searchWindow, int kernelSize, float sigma_r)
{
    //protected values are assigned/computed
    this->sigma_r = sigma_r;
    sigma_r_sq_2 = sigma_r * sigma_r * 2.0f;

    int halfKernelSize = kernelSize >> 1;
    this->kernelSize = (halfKernelSize << 1) + 1;
    kernelSize_sq = float(this->kernelSize * this->kernelSize);

    h = sigma_r * 0.5f;
    h_sq = h * h;

    halfSearchWindow = (searchWindow >> 1);
    this->searchWindow = halfSearchWindow << 1;

    seed = 1;
}

PIC_INLINE void FilterNonLocalMeansF::ProcessBBox(Image *dst, ImageVec src,
        BBox *box)
{
    int width = dst->width;
    int height = dst->height;
    int channels = dst->channels;

    PatchComp pc(src[0], src[0], kernelSize);

    float area = kernelSize_sq * dst->channelsf;

    for(int j = box->y0; j < box->y1; j++) {
        for(int i = box->x0; i < box->x1; i++) {

            float *tmp_dst = (*dst)(i, j);

            float tot = 0.0f;
            Array<float>::assign(0.0f, tmp_dst, dst->channels);

            for(int l = -halfSearchWindow; l <= halfSearchWindow; l++) {
                for(int m = -halfSearchWindow; m <= halfSearchWindow; m++) {
                    //fetch addresses
                    int ci = CLAMP(i + m,  width);
                    int cj = CLAMP(j + l, height);

                    float *tmp_src = (*src[0])(ci, cj);

                    float d_sq = pc.getSSD(i, j, ci, cj) / area;

                    float w = expf(-MAX(d_sq - sigma_r_sq_2, 0.0f) / h_sq);
                    tot += w;

                    for(int c = 0; c < channels; c++) {
                        tmp_dst[c] += tmp_src[c] * w;
                    }
                }
            }

            float *tmp_src = (*src[0])(i, j);
            bool sumTest = tot > 0.0f;

            for(int c = 0; c < channels; c++) {
                tmp_dst[c] = sumTest ? tmp_dst[c] / tot : tmp_src[c];
            }
        }
    }
}

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_NON_LOCAL_MEANS_F_HPP */

