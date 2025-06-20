﻿/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/OpusCodec.h"

/** @file PhotonVoice-cpp/inc/OpusCodec.h */

namespace ExitGames
{
	namespace Voice
	{
		namespace Opus
		{
			using namespace Common;
			using namespace Common::MemoryManagement;

			template<typename T>
			Encoder<T>::Encoder(const VoiceInfo& info, const Voice::Logger& logger)
				: mpEncoder(NULL)
				, mpOutputOpaque(NULL)
				, mpOutput(NULL)
				, mLogger(logger)
				, mChannels(info.getChannels())
				, mOutBuf(RECOMMENDED_MAX_PACKET_SIZE)
			{
				int err;
				mpEncoder = opus_encoder_create(info.getSamplingRate(), info.getChannels(), OPUS_APPLICATION_VOIP, &err);
				if(err)
				{
					mError = JString(L"Encoder: creation error: ") + opus_strerror(err) + L" (" + err + L")";
					EGLOG(DebugLevel::ERRORS, JString(L"[PV] ") + mError);
				}
			}

			template<typename T>
			Encoder<T>::~Encoder(void)
			{
				close();
			}

			template<typename T>
			void Encoder<T>::close(void)
			{
				if(mpEncoder)
				{
					opus_encoder_destroy(mpEncoder);
					mpEncoder = NULL;
				}
			}

			template<typename T>
			JString Encoder<T>::getError(void) const
			{
				return mError;
			}

			template<typename T>
			static opus_int32 opus_encode(OpusEncoder* st, const T* pcm, int frame_size, unsigned char* data, opus_int32 max_data_bytes);

			template<>
			opus_int32 opus_encode<float>(OpusEncoder* st, const float* pcm, int frame_size, unsigned char* data, opus_int32 max_data_bytes)
			{
				return ::opus_encode_float(st, pcm, frame_size, data, max_data_bytes);
			}

			template<>
			opus_int32 opus_encode<short>(OpusEncoder* st, const opus_int16* pcm, int frame_size, unsigned char* data, opus_int32 max_data_bytes)
			{
				return ::opus_encode(st, pcm, frame_size, data, max_data_bytes);
			}

			template<typename T>
			void Encoder<T>::setOutput(void* opaque, void(*output)(void*, const Buffer<nByte>&, FrameFlags::Enum flags))
			{
				mpOutputOpaque = opaque;
				mpOutput = output;
			}

			Buffer<nByte> emptyBuffer;

			template<typename T>
			Buffer<nByte> Encoder<T>::dequeueOutput(FrameFlags::Enum& /*flags*/)
			{
				return emptyBuffer;
			}

			template<typename T>
			void Encoder<T>::input(const Buffer<T>& buf)
			{
				if(getError().length())
					return;

				if(!mpOutput)
				{
					mError = "OpusCodec.Encoder: Output action is not set";
					return;
				}

				int outSamples = opus_encode<T>(mpEncoder, buf.getArray(), buf.getSize()/mChannels, mOutBuf.getArray(), mOutBuf.getSize());
				if(outSamples > 0)
				{
					Buffer<nByte> b(outSamples);
					memcpy(b.getArray(), mOutBuf.getArray(), outSamples);
					mpOutput(mpOutputOpaque, b, FrameFlags::None);
				}
				else if(!outSamples)
					;
				else
					EGLOG(DebugLevel::ERRORS, JString("[PV] Encoder: EncodeAndGetOutput error: ") + opus_strerror(outSamples) + L" (" + outSamples + L")");
			}

			template <typename T>
			Decoder<T>::Decoder(void* outputOpaque, void(*output)(void*, const Buffer<T>&), const Voice::Logger& logger)
				: mpDecoder(NULL)
				, mpOutputOpaque(outputOpaque)
				, mpOutput(output)
				, mLogger(logger)
			{
			}

			template <typename T>
			Decoder<T>::~Decoder(void)
			{
				close();
			}

			template <typename T>
			void Decoder<T>::open(const VoiceInfo& info)
			{
				mInfo = info;
				int err;
				mpDecoder = opus_decoder_create(mInfo.getSamplingRate(), mInfo.getChannels(), &err);
				if(err)
				{
					mError = JString(L"Decoder: creation error: ") + opus_strerror(err) + L" (" + err + L")";
					EGLOG(DebugLevel::ERRORS, JString(L"[PV] ") + mError);
				}
			}

			template <typename T>
			void Decoder<T>::close(void)
			{
				if(mpDecoder)
				{
					opus_decoder_destroy(mpDecoder);
					mpDecoder = NULL;
				}
			}

			template <typename T>
			JString Decoder<T>::getError(void) const
			{
				return mError;
			}

			template <typename T>
			void Decoder<T>::input(const FrameBuffer& buf)
			{
				if(mError.length())
					return;

				Buffer<T> out(mInfo.getFrameSize());

				int outSamples = decodeTyped(buf, out.getArray());

				if(outSamples > 0)
					mpOutput(mpOutputOpaque, out);
				else if(!outSamples)
					;
				else
					EGLOG(DebugLevel::ERRORS, JString("[PV] Decoder: decodeToFloat error: ") + opus_strerror(outSamples) + L" (" + outSamples + L")");
			}

			template<> int Decoder<float>::decodeTyped(const Buffer<nByte>& buf, float* outArray)
			{
				return opus_decode_float(mpDecoder, buf.getArray(), buf.getSize(), outArray, mInfo.getFrameDurationSamples(), 0);
			}

			template<> int Decoder<short>::decodeTyped(const Buffer<nByte>& buf, short* outArray)
			{
				return opus_decode(mpDecoder, buf.getArray(), buf.getSize(), outArray, mInfo.getFrameDurationSamples(), 0);
			}

			// Opus supports only float and short buffers
			template class Encoder<float>;
			template class Encoder<short>;
			template class Decoder<float>;
			template class Decoder<short>;
		}
	}
}