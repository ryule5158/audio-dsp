/*
 * Diagnostic-only translation unit used while sizing opaque public contexts.
 * It is deliberately excluded from CMake because including every upstream
 * project header together exposes generated macro-name collisions that do not
 * exist in normal per-module translation units.
 */
#include "braids/macro_oscillator.h"
#include "clouds/dsp/granular_processor.h"
#include "elements/dsp/part.h"
#include "frames/keyframer.h"
#include "frames/poly_lfo.h"
#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"
#include "peaks/processors.h"
#include "plaits/dsp/voice.h"
#include "rings/dsp/part.h"
#include "stages/segment_generator.h"
#include "streams/processor.h"
#include "tides/generator.h"
#include "tides2/poly_slope_generator.h"
#include "warps/dsp/modulator.h"

extern "C" {
char aud_euro_size_braids[sizeof(braids::MacroOscillator)];
char aud_euro_size_clouds[sizeof(clouds::GranularProcessor)];
char aud_euro_size_elements[sizeof(elements::Part)];
char aud_euro_size_frames_keyframer[sizeof(frames::Keyframer)];
char aud_euro_size_frames_lfo[sizeof(frames::PolyLfo)];
char aud_euro_size_marbles_t[sizeof(marbles::TGenerator)];
char aud_euro_size_marbles_xy[sizeof(marbles::XYGenerator)];
char aud_euro_size_peaks[sizeof(peaks::Processors)];
char aud_euro_size_plaits[sizeof(plaits::Voice)];
char aud_euro_size_rings[sizeof(rings::Part)];
char aud_euro_size_stages[sizeof(stages::SegmentGenerator)];
char aud_euro_size_streams[sizeof(streams::Processor)];
char aud_euro_size_tides[sizeof(tides::Generator)];
char aud_euro_size_tides2[sizeof(tides::PolySlopeGenerator)];
char aud_euro_size_warps[sizeof(warps::Modulator)];
}
