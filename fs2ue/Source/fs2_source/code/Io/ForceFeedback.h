#ifndef __FORCEFEEDBACK_H__
#define __FORCEFEEDBACK_H__

#ifndef UNITY_BUILD
#include "Joy_ff.h"
#include <assert.h>
#include "timer.h"
#endif

class ForceFeedback
{
public:
	static const int MAX_MOTORS                 = 2;
	static const int MAX_CONTROLLERS            = 4;
	static const int MAX_RUMBLE                 = 65535;
	static const int MAX_EFFECTS_PER_CONTROLLER = 5;

	static const int ONE_SEC = 1000;

	class Effect
	{
	public:
		enum
		{
			LINEAR_DECREASE,
			LINEAR_INCREASE_HOLD,
			CONSTANT,
		};

		Effect();
		void Start(const int motor, const int duration, const int graph, const int peak);

		void EndEffect();
		bool IsActive()  {return m_active;}
		int  GetRumble() {return m_rumble;}
		int  GetMotor()  {return m_motor;}

		void Update(float frameTimeMS);

	private:
		float  m_timeInPlay;
		float  m_duration;
		int  m_motor;
		int  m_graph;
		int  m_rumble;
		int  m_peak;
		bool m_active;
	};

	void Start(const int motor, const int graph, const int duration, const int timeInPlay, const int peak);
	Effect &StartEffect(const int padNum, const int motor, const int graph, const int duration, const int peak);
	void EndEffects(int padNum, float fadeTime);
	void Update(float frameTimeMS);

private:

	Effect m_Effects[MAX_CONTROLLERS][MAX_EFFECTS_PER_CONTROLLER];

	int m_type, m_initType;

public:

	void Init(int type);
	void Shutdown(int type);
	void StopEffects();
	void MissionInit(vector v);
	void Reacquire();
	void Unacquire();
	void PlayVectorEffect(vector *v, float scaler);
	void PlayDirEffect(float x, float y);
	void PlayPrimaryShoot(int gain);
	void PlaySecondaryShoot(int gain);
	void AdjustHandling(int speed);
	void Docked();
	void PlayReloadEffect();
	void AfterburnOn();
	void AfterburnOff();
	void Explode();
	void FlyBy(int mag);
	void Deathroll();
};

extern ForceFeedback g_ForceFeedback;

#endif
