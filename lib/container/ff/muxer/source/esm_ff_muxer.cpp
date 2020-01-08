#include "mgnt_ff_muxer_v4.2.1.h"
#include "ff_muxer_v4.2.1.h"

magnetar::lib::container::ff::v4_2_1::muxer::muxer(void)
{
	_core = new magnetar::lib::container::ff::v4_2_1::muxer::core(this);
}

magnetar::lib::container::ff::v4_2_1::muxer::~muxer(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

bool magnetar::lib::container::ff::v4_2_1::muxer::is_initialized(void)
{
	return _core->is_initialized();
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::initialize(magnetar::lib::container::ff::v4_2_1::muxer::context_t * context)
{
	return _core->initialize(context);
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::release(void)
{
	return _core->release();
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::put_video_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts)
{
	return _core->put_video_stream(bytes, nbytes, dts, cts);
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::put_audio_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts)
{
	return _core->put_audio_stream(bytes, nbytes, dts, cts);
}