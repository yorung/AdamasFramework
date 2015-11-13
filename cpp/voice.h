class Voice
{
	struct WaveContext* context = nullptr;
	bool IsReady() { return !!context; }
public:
	Voice(const char *fileName) { Create(fileName); }
	~Voice() { Destroy(); }
	void Create(const char *fileName);
	void Play(bool loop);
	void Stop();
	void Destroy();
};
