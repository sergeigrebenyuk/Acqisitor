
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MCTRANSFER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MCTRANSFER_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MCTRANSFER_EXPORTS
#define MCTRANSFER_API __declspec(dllexport)
#else
#define MCTRANSFER_API __declspec(dllimport)
#endif

// This class is exported from the MCTransfer.dll
class MCTRANSFER_API CMCTransfer {
public:
	CMCTransfer(void);
	// TODO: add your methods here.
};

extern MCTRANSFER_API int nMCTransfer;

MCTRANSFER_API int fnMCTransfer(void);
/*
MCTRANSFER_API int		Test(UINT t);
MCTRANSFER_API void		StartPump();
MCTRANSFER_API void		StopPump();
MCTRANSFER_API void		Reset();
MCTRANSFER_API void		_stdcall	TossMCPacket(long buf);
MCTRANSFER_API int		Connect(UINT SN, UINT Stage);
MCTRANSFER_API int		SelectMC(UINT StageIdx);
MCTRANSFER_API int		SetMode(UINT StageIdx, UINT Mode);

MCTRANSFER_API int		GetMode(UINT StageIdx);
MCTRANSFER_API double	GetPrimaryScaleIn(UINT StageIdx);
MCTRANSFER_API double	GetSecondaryScaleIn(UINT StageIdx);
MCTRANSFER_API double	GetScaleOut(UINT StageIdx);
MCTRANSFER_API char		GetStatus(UINT StageIdx);

MCTRANSFER_API int		AutoPipetteOffset();
MCTRANSFER_API int		SetPipetteOffset( double Offs);
MCTRANSFER_API int		AutoFastCompensation();
MCTRANSFER_API int		AutoSlowCompensation();
MCTRANSFER_API int		AutoWholeCellCompensation();
MCTRANSFER_API int		AutoLeakSubst();
*/