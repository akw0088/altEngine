//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "ent_speaker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EntSpeaker::EntSpeaker(Entity *entity, Audio &audio)
{
	EntSpeaker::entity = entity;

	source = audio.create_source(false, false);
	loop_source = audio.create_source(true, false);

	index = -1;

	//audio.effects(loop_source);

	audio.play(loop_source);
}

void EntSpeaker::gain(float value)
{
#ifndef DEDICATED
	alSourcef(source, AL_GAIN, value);
#endif
}

void EntSpeaker::loop_gain(float value)
{
#ifndef DEDICATED
	alSourcef(loop_source, AL_GAIN, value);
#endif
}

void EntSpeaker::destroy(Audio &audio)
{
	if (source >= 0)
		audio.delete_source(source);

	if (loop_source >= 0)
		audio.delete_source(loop_source);

}

EntSpeaker::~EntSpeaker()
{
	source = -1;
}
