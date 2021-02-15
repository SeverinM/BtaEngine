#ifndef H_EVENT_READER_GPU
#define H_EVENT_READER_GPU

namespace Bta
{
	namespace Graphic
	{
		class EventReaderGPU
		{
			public:
				static void Update();
			private:
				static void CheckNewComponent();
				static void CheckVerticeBufferChange();
		};
	}
}

#endif H_EVENT_READER_GPU
