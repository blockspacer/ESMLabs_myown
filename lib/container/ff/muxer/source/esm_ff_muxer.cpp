#include "esm_ff_muxer.h"
#include "ff_muxer.h"

esmlabs::lib::container::ff::muxer::muxer(void)
{
	_core = new esmlabs::lib::container::ff::muxer::core(this);
}

esmlabs::lib::container::ff::muxer::~muxer(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

bool esmlabs::lib::container::ff::muxer::is_initialized(void)
{
	return _core->is_initialized();
}

int32_t esmlabs::lib::container::ff::muxer::initialize(esmlabs::lib::container::ff::muxer::context_t * context)
{
	return _core->initialize(context);
}

int32_t esmlabs::lib::container::ff::muxer::release(void)
{
	return _core->release();
}

int32_t esmlabs::lib::container::ff::muxer::put_video_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts)
{
	return _core->put_video_stream(bytes, nbytes, dts, cts);
}

int32_t esmlabs::lib::container::ff::muxer::put_audio_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts)
{
	return _core->put_audio_stream(bytes, nbytes, dts, cts);
}