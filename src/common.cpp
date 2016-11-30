// Copyright (c) 2016 Danilo Peixoto. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "common.h"

#include <cmath>
#include <numeric>

float min(float a, float b) {
	return a > b ? b : a;
}

float max(float a, float b) {
	return a < b ? b : a;
}

float clamp(float x, float x0, float x1) {
	return max(x0, min(x1, x));
}

float remap(float x, float x0, float x1, float y0, float y1) {
	return y0 + (y1 - y0) / (x1 - x0) * (x - x0);
}

float degrees(float radians) {
    return radians * 180.0 / PI;
}

float radians(float degrees) {
    return degrees * PI / 180.0;
}

float mean(const vector<float> & data) {
    size_t size = data.size();

    return size != 0 ? accumulate(data.begin(), data.end(), (float)0) / size : 0;
}

float standardDeviation(const vector<float> & data) {
    size_t size = data.size();

    if (size == 0 || size == 1) {
        return 0;
    }

    float average, sum, deviation;
    vector<float>::const_iterator dataIt;

    average = mean(data);
    sum = 0;

    for (dataIt = data.begin(); dataIt != data.end(); dataIt++) {
        deviation = *dataIt - average;
        sum += deviation * deviation;
    }

    return sqrt(sum / (size - 1));
}
