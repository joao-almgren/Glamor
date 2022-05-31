#pragma once
#include <memory>

class Timer;

class FpsCounter
{
public:
	FpsCounter() noexcept;
	void tick() noexcept;
	double getFps() const noexcept;
	double getAverageFps() const noexcept;
	unsigned int getFrameCount() const noexcept;

private:
	double fps, averageFps;
	unsigned int frameCount;
	std::shared_ptr<Timer> timer;
};
