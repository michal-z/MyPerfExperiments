#pragma once

struct IExperiment
{
	void(*Update)();
	bool(*Initialize)();
	void(*Shutdown)();
};

IExperiment Experiment0();
