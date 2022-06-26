#ifndef VS_STREAM_SVCTYPES_H
#define VS_STREAM_SVCTYPES_H

#define SVC_NONE		0x00000000
#define SVC_2T_MODE		0x00000100	/// 2 temporal
#define SVC_3T_MODE		0x00000200	/// 3 temporal
#define SVC_2S_MODE		0x00010000	/// 2 spatial
#define SVC_3S_MODE		0x00020000	/// 3 spatial
#define SVC_SMOD_MODE	0x00040000	/// decrease MB threshold spatial layers

#define SVC_2S_2T_MODE	(SVC_2S_MODE | SVC_2T_MODE)
#define SVC_3S_2T_MODE	(SVC_3S_MODE | SVC_2S_MODE | SVC_2T_MODE)
#define SVC_M3S_2T_MODE	(SVC_SMOD_MODE | SVC_3S_MODE | SVC_2S_MODE | SVC_2T_MODE)

#endif /* VS_STREAM_SVCTYPES_H */