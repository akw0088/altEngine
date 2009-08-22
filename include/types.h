typedef struct
{
	short	format;
	short	channels;
	int	sampleRate;
	int	avgSampleRate;
	short	align;
	short	sampleSize;
} waveFormat_t;

typedef struct
{
	waveFormat_t	*format;
	void		*pcmData;
	int		dataSize;
	int		duration;
	char		*file;
} wave_t;


typedef struct
{
	float	left[4];
	float	right[4];
	float	top[4];
	float	bottom[4];
	float	zNear[4];
	float	zFar[4];
} frustum_t;

