class Voice
{
	struct WaveContext
	{
		HWAVEOUT hWaveOut;
		WAVEHDR wavehdr;
		void *fileImg;
	};
	WaveContext context;
	bool IsReady();
public:
	Voice();
	Voice(const char *fileName);
	~Voice();
	void Load(const char *fileName);
	void Play(bool loop);
	void Stop();
	void Release();
};
