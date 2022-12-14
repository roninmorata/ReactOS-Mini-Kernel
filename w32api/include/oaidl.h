#ifndef _OAIDL_H
#define _OAIDL_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if (__STDC__ && !defined(_FORCENAMELESSUNION)) || defined(NONAMELESSUNION)
#define __VARIANT_NAME_1 n1
#define __VARIANT_NAME_2 n2
#define __VARIANT_NAME_3 n3
#define __VARIANT_NAME_4 brecVal
#else
#define __tagVARIANT
#define __tagBRECORD
#define __VARIANT_NAME_1
#define __VARIANT_NAME_2
#define __VARIANT_NAME_3
#define __VARIANT_NAME_4
#endif
#define DISPID_UNKNOWN (-1)
#define DISPID_VALUE (0)
#define DISPID_PROPERTYPUT (-3)
#define DISPID_NEWENUM (-4)
#define DISPID_EVALUATE (-5)
#define DISPID_CONSTRUCTOR (-6)
#define DISPID_DESTRUCTOR (-7)
#define DISPID_COLLECT (-8)
#define FADF_AUTO (1)
#define FADF_STATIC (2)
#define FADF_EMBEDDED (4)
#define FADF_FIXEDSIZE (16)
#define FADF_RECORD (32)
#define FADF_HAVEIID (64)
#define FADF_HAVEVARTYPE (128)
#define FADF_BSTR (256)
#define FADF_UNKNOWN (512)
#define FADF_DISPATCH (1024)
#define FADF_VARIANT (2048)
#define FADF_RESERVED (0xf0e8)
#define FADF_DATADELETED (0x1000)
#define FADF_CREATEVECTOR (0x2000)
#define PARAMFLAG_NONE (0)
#define PARAMFLAG_FIN (1)
#define PARAMFLAG_FOUT (2)
#define PARAMFLAG_FLCID (4)
#define PARAMFLAG_FRETVAL (8)
#define PARAMFLAG_FOPT (16)
#define PARAMFLAG_FHASDEFAULT (32)
#define IDLFLAG_NONE PARAMFLAG_NONE
#define IDLFLAG_FIN PARAMFLAG_FIN
#define IDLFLAG_FOUT PARAMFLAG_FOUT
#define IDLFLAG_FLCID PARAMFLAG_FLCID
#define IDLFLAG_FRETVAL PARAMFLAG_FRETVAL
#define IMPLTYPEFLAG_FDEFAULT 1
#define IMPLTYPEFLAG_FSOURCE 2
#define IMPLTYPEFLAG_FRESTRICTED 4
#define IMPLTYPEFLAG_FDEFAULTVTABLE 8

typedef interface ITypeLib *LPTYPELIB;
typedef interface ITypeLib2 *LPTYPELIB2;
typedef interface ICreateTypeInfo *LPCREATETYPEINFO;
typedef interface ICreateTypeInfo2 *LPCREATETYPEINFO2;
typedef interface ICreateTypeLib *LPCREATETYPELIB;
typedef interface ICreateTypeLib2 *LPCREATETYPELIB2;
typedef interface ITypeComp *LPTYPECOMP;
typedef interface ITypeInfo *LPTYPEINFO;
typedef interface ITypeInfo2 *LPTYPEINFO2;
typedef interface IErrorInfo *LPERRORINFO;
typedef interface IDispatch *LPDISPATCH;
typedef interface IEnumVARIANT *LPENUMVARIANT;
typedef interface ICreateErrorInfo *LPCREATEERRORINFO;
typedef interface ISupportErrorInfo *LPSUPPORTERRORINFO;
typedef interface IRecordInfo *LPRECORDINFO;

extern const IID IID_ITypeLib;
extern const IID IID_ITypeLib2;
extern const IID IID_ICreateTypeInfo;
extern const IID IID_ICreateTypeInfo2;
extern const IID IID_ICreateTypeLib;
extern const IID IID_ICreateTypeLib2;
extern const IID IID_ITypeInfo;
extern const IID IID_ITypeInfo2;
extern const IID IID_IErrorInfo;
extern const IID IID_IDispatch;
extern const IID IID_IEnumVARIANT;
extern const IID IID_ICreateErrorInfo;
extern const IID IID_ISupportErrorInfo;
extern const IID IID_IRecordInfo;
extern const IID IID_ITypeMarshal;

typedef enum tagSYSKIND {
	SYS_WIN16,SYS_WIN32,SYS_MAC
} SYSKIND;
typedef enum tagLIBFLAGS {
	LIBFLAG_FRESTRICTED=1,LIBFLAG_FCONTROL=2,LIBFLAG_FHIDDEN=4,
	LIBFLAG_FHASDISKIMAGE=8
} LIBFLAGS;
typedef struct tagTLIBATTR {
	GUID guid;
	LCID lcid;
	SYSKIND syskind;
	WORD wMajorVerNum;
	WORD wMinorVerNum;
	WORD wLibFlags;
} TLIBATTR,*LPTLIBATTR;
typedef CY CURRENCY;
typedef struct tagSAFEARRAYBOUND {
	ULONG cElements;
	LONG lLbound;
}SAFEARRAYBOUND,*LPSAFEARRAYBOUND;
typedef struct _wireSAFEARR_BSTR
{
	ULONG Size;
	wireBSTR *aBstr;
}SAFEARR_BSTR;
typedef struct _wireSAFEARR_UNKNOWN {
	ULONG Size;
	IUnknown **apUnknown;
}SAFEARR_UNKNOWN;
typedef struct _wireSAFEARR_DISPATCH {
	ULONG Size;
	LPDISPATCH *apDispatch;
}SAFEARR_DISPATCH;
typedef struct _wireSAFEARR_VARIANT {
	ULONG Size;
	struct _wireVARIANT *aVariant;
}SAFEARR_VARIANT;
typedef enum tagSF_TYPE {
	SF_ERROR=VT_ERROR,
	SF_I1=VT_I1,
	SF_I2=VT_I2,
	SF_I4=VT_I4,
	SF_I8=VT_I8,
	SF_BSTR=VT_BSTR,
	SF_UNKNOWN=VT_UNKNOWN,
	SF_DISPATCH=VT_DISPATCH,
	SF_VARIANT=VT_VARIANT
}SF_TYPE;
typedef struct _wireBRECORD  {
	ULONG fFlags;
	ULONG clSize;
	LPRECORDINFO* pRecInfo;
	byte* pRecord;
} *wireBRECORD;
typedef struct _wireSAFEARR_BRECORD {
    ULONG Size;
    wireBRECORD* aRecord;
    } SAFEARR_BRECORD;
typedef struct _wireSAFEARR_HAVEIID {
	ULONG Size;
	IUnknown** apUnknown;
	IID iid;
	} SAFEARR_HAVEIID;
typedef struct _wireSAFEARRAY_UNION {
	ULONG sfType;
	union {
		SAFEARR_BSTR BstrStr;
		SAFEARR_UNKNOWN UnknownStr;
		SAFEARR_DISPATCH DispatchStr;
		SAFEARR_VARIANT VariantStr;
		SAFEARR_BRECORD RecordStr;
		SAFEARR_HAVEIID HaveIidStr;
		BYTE_SIZEDARR ByteStr;
		WORD_SIZEDARR WordStr;
		DWORD_SIZEDARR LongStr;
		HYPER_SIZEDARR HyperStr;
	}u;
}SAFEARRAYUNION;
typedef struct _wireSAFEARRAY {
	USHORT cDims;
	USHORT fFeatures;
	ULONG cbElements;
	ULONG cLocks;
	SAFEARRAYUNION uArrayStructs;
	SAFEARRAYBOUND rgsabound[1];
}*wireSAFEARRAY;
typedef wireSAFEARRAY *wirePSAFEARRAY;
typedef struct tagSAFEARRAY {
	USHORT cDims;
	USHORT fFeatures;
	ULONG cbElements;
	ULONG cLocks;
	PVOID pvData;
	SAFEARRAYBOUND rgsabound[1];
}SAFEARRAY,*LPSAFEARRAY;
#if defined (__GNUC__) && !defined (NONAMELESSUNION)
__extension__   /* no named members  */
#endif
typedef struct tagVARIANT {
  _ANONYMOUS_UNION union {
	struct __tagVARIANT {
	VARTYPE vt;
	WORD wReserved1;
	WORD wReserved2;
	WORD wReserved3;
	_ANONYMOUS_UNION union {
		long lVal;
		LONGLONG llVal;
		unsigned char bVal;
		short iVal;
		float fltVal;
		double dblVal;
		VARIANT_BOOL  boolVal;
		SCODE scode;
		CY cyVal;
		DATE date;
		BSTR bstrVal;
		IUnknown *punkVal;
		LPDISPATCH pdispVal;
		SAFEARRAY *parray;
		unsigned char *pbVal;
		short *piVal;
		long *plVal;
		float *pfltVal;
		double *pdblVal;
		VARIANT_BOOL *pboolVal;
		_VARIANT_BOOL  *pbool;
		SCODE *pscode;
		CY *pcyVal;
		DATE *pdate;
		BSTR *pbstrVal;
		IUnknown **ppunkVal;
		LPDISPATCH *ppdispVal;
		SAFEARRAY **pparray;
		struct tagVARIANT *pvarVal;
		void *byref;
		CHAR cVal;
		USHORT uiVal;
		ULONG ulVal;
		ULONGLONG ullVal;
		INT intVal;
		UINT uintVal;
		DECIMAL *pdecVal;
		CHAR  *pcVal;
		USHORT  *puiVal;
		ULONG  *pulVal;
		INT  *pintVal;
		UINT  *puintVal;
		_ANONYMOUS_STRUCT struct __tagBRECORD {
			PVOID pvRecord;
			struct IRecordInfo *pRecInfo;
		} __VARIANT_NAME_4;
	} __VARIANT_NAME_3;
    } __VARIANT_NAME_2;
    DECIMAL decVal;
  } __VARIANT_NAME_1;
} VARIANT,*LPVARIANT;
typedef VARIANT VARIANTARG;
typedef VARIANT *LPVARIANTARG;
typedef struct _wireVARIANT {
	DWORD clSize;
	DWORD rpcReserved;
	USHORT vt;
	USHORT wReserved1;
	USHORT wReserved2;
	USHORT wReserved3;
	_ANONYMOUS_UNION union {
		LONG lVal;
		LONGLONG llVal;
		BYTE bVal;
		SHORT iVal;
		FLOAT fltVal;
		DOUBLE dblVal;
		VARIANT_BOOL boolVal;
		SCODE scode;
		CY cyVal;
		DATE date;
		wireBSTR bstrVal;
		IUnknown *punkVal;
		LPDISPATCH pdispVal;
		wirePSAFEARRAY parray;
		wireBRECORD brecVal;
		BYTE *pbVal;
		SHORT *piVal;
		LONG *plVal;
		FLOAT *pfltVal;
		DOUBLE *pdblVal;
		VARIANT_BOOL *pboolVal;
		SCODE *pscode;
		CY *pcyVal;
		DATE *pdate;
		wireBSTR *pbstrVal;
		IUnknown **ppunkVal;
		LPDISPATCH *ppdispVal;
		wirePSAFEARRAY *pparray;
		struct _wireVARIANT *pvarVal;
		CHAR cVal;
		USHORT uiVal;
		ULONG ulVal;
		ULONGLONG ullVal;
		INT intVal;
		UINT uintVal;
		DECIMAL decVal;
		DECIMAL *pdecVal;
		CHAR *pcVal;
		USHORT *puiVal;
		ULONG *pulVal;
		INT *pintVal;
		UINT *puintVal;
	} DUMMYUNIONNAME;
} *wireVARIANT;  
typedef LONG DISPID;
typedef DISPID MEMBERID;
typedef DWORD HREFTYPE;
typedef enum tagTYPEKIND {
	TKIND_ENUM,TKIND_RECORD,TKIND_MODULE,TKIND_INTERFACE,TKIND_DISPATCH,
	TKIND_COCLASS,TKIND_ALIAS,TKIND_UNION,TKIND_MAX
}TYPEKIND;
typedef struct tagTYPEDESC {
	_ANONYMOUS_UNION union {
		struct tagTYPEDESC *lptdesc;
		struct tagARRAYDESC *lpadesc;
		HREFTYPE hreftype;
	} DUMMYUNIONNAME;
	VARTYPE vt;
}TYPEDESC;
typedef struct tagARRAYDESC {
	TYPEDESC tdescElem;
	USHORT cDims;
	SAFEARRAYBOUND rgbounds[1];
}ARRAYDESC;
typedef struct tagPARAMDESCEX {
	ULONG cBytes;
	VARIANTARG varDefaultValue;
}PARAMDESCEX,*LPPARAMDESCEX;
typedef struct tagPARAMDESC {
	LPPARAMDESCEX pparamdescex;
	USHORT wParamFlags;
}PARAMDESC,*LPPARAMDESC;
typedef struct tagIDLDESC {
	ULONG dwReserved;
	USHORT wIDLFlags;
}IDLDESC,*LPIDLDESC;
typedef struct tagELEMDESC {
	TYPEDESC tdesc;
	_ANONYMOUS_UNION union {
		IDLDESC idldesc;
		PARAMDESC paramdesc;
	} DUMMYUNIONNAME;
} ELEMDESC,*LPELEMDESC;
typedef struct tagTYPEATTR {
	GUID guid;
	LCID lcid;
	DWORD dwReserved;
	MEMBERID memidConstructor;
	MEMBERID memidDestructor;
	LPOLESTR lpstrSchema;
	ULONG cbSizeInstance;
	TYPEKIND typekind;
	WORD cFuncs;
	WORD cVars;
	WORD cImplTypes;
	WORD cbSizeVft;
	WORD cbAlignment;
	WORD wTypeFlags;
	WORD wMajorVerNum;
	WORD wMinorVerNum;
	TYPEDESC tdescAlias;
	IDLDESC idldescType;
}TYPEATTR,*LPTYPEATTR;
typedef struct tagDISPPARAMS {
	VARIANTARG *rgvarg;
	DISPID *rgdispidNamedArgs;
	UINT cArgs;
	UINT cNamedArgs;
}DISPPARAMS;
typedef struct tagEXCEPINFO {
	WORD wCode;
	WORD wReserved;
	BSTR bstrSource;
	BSTR bstrDescription;
	BSTR bstrHelpFile;
	DWORD dwHelpContext;
	PVOID pvReserved;
	HRESULT(__stdcall * pfnDeferredFillIn)(struct tagEXCEPINFO*);
	SCODE scode;
} EXCEPINFO,*LPEXCEPINFO;
typedef enum tagCALLCONV {
	CC_FASTCALL,CC_CDECL,CC_MSCPASCAL,CC_PASCAL=CC_MSCPASCAL,
	CC_MACPASCAL,CC_STDCALL,CC_FPFASTCALL,CC_SYSCALL,CC_MPWCDECL,
	CC_MPWPASCAL,CC_MAX=CC_MPWPASCAL
}CALLCONV;
typedef enum tagFUNCKIND {
	FUNC_VIRTUAL,FUNC_PUREVIRTUAL,FUNC_NONVIRTUAL,
	FUNC_STATIC,FUNC_DISPATCH
}FUNCKIND;
typedef enum tagINVOKEKIND {
	INVOKE_FUNC=1,INVOKE_PROPERTYGET,INVOKE_PROPERTYPUT=4,
	INVOKE_PROPERTYPUTREF=8
}INVOKEKIND;
typedef struct tagFUNCDESC {
	MEMBERID memid;
	SCODE *lprgscode;
	ELEMDESC *lprgelemdescParam;
	FUNCKIND funckind;
	INVOKEKIND invkind;
	CALLCONV callconv;
	SHORT cParams;
	SHORT cParamsOpt;
	SHORT oVft;
	SHORT cScodes;
	ELEMDESC elemdescFunc;
	WORD wFuncFlags;
}FUNCDESC,*LPFUNCDESC;
typedef enum tagVARKIND {
	VAR_PERINSTANCE,VAR_STATIC,VAR_CONST,VAR_DISPATCH
} VARKIND;
typedef struct tagVARDESC {
	MEMBERID memid;
	LPOLESTR lpstrSchema;
	_ANONYMOUS_UNION union {
		ULONG oInst;
		VARIANT *lpvarValue;
	} DUMMYUNIONNAME;
	ELEMDESC elemdescVar;
	WORD wVarFlags;
	VARKIND varkind;
} VARDESC,*LPVARDESC;
typedef enum tagTYPEFLAGS {
	TYPEFLAG_FAPPOBJECT=1,TYPEFLAG_FCANCREATE=2,TYPEFLAG_FLICENSED=4,
	TYPEFLAG_FPREDECLID=8,TYPEFLAG_FHIDDEN=16,TYPEFLAG_FCONTROL=32,
	TYPEFLAG_FDUAL=64,TYPEFLAG_FNONEXTENSIBLE=128,
	TYPEFLAG_FOLEAUTOMATION=256,TYPEFLAG_FRESTRICTED=512,
	TYPEFLAG_FAGGREGATABLE=1024,TYPEFLAG_FREPLACEABLE=2048,
	TYPEFLAG_FDISPATCHABLE=4096,TYPEFLAG_FREVERSEBIND=8192,
	TYPEFLAG_FPROXY=16384
} TYPEFLAGS;
typedef enum tagFUNCFLAGS {
	FUNCFLAG_FRESTRICTED=1,FUNCFLAG_FSOURCE=2,FUNCFLAG_FBINDABLE=4,
	FUNCFLAG_FREQUESTEDIT=8,FUNCFLAG_FDISPLAYBIND=16,FUNCFLAG_FDEFAULTBIND=32,
	FUNCFLAG_FHIDDEN=64,FUNCFLAG_FUSESGETLASTERROR=128,FUNCFLAG_FDEFAULTCOLLELEM=256,
	FUNCFLAG_FUIDEFAULT=512,FUNCFLAG_FNONBROWSABLE=1024,FUNCFLAG_FREPLACEABLE=2048,
	FUNCFLAG_FIMMEDIATEBIND=4096
} FUNCFLAGS;
typedef enum tagVARFLAGS {
	VARFLAG_FREADONLY=1,VARFLAG_FSOURCE=2,VARFLAG_FBINDABLE=4,VARFLAG_FREQUESTEDIT=8,
	VARFLAG_FDISPLAYBIND=16,VARFLAG_FDEFAULTBIND=32,VARFLAG_FHIDDEN=64,VARFLAG_FRESTRICTED=128,
	VARFLAG_FDEFAULTCOLLELEM=256,VARFLAG_FUIDEFAULT=512,VARFLAG_FNONBROWSABLE=1024,
	VARFLAG_FREPLACEABLE=2048,VARFLAG_FIMMEDIATEBIND=4096
} VARFLAGS;
typedef struct tagCLEANLOCALSTORAGE {
	IUnknown *pInterface;
	PVOID pStorage;
	DWORD flags;
} CLEANLOCALSTORAGE;
typedef struct tagCUSTDATAITEM {
	GUID guid;
	VARIANTARG varValue;
} CUSTDATAITEM,*LPCUSTDATAITEM;
typedef struct tagCUSTDATA {
	DWORD cCustData;
	LPCUSTDATAITEM prgCustData;
} CUSTDATA,*LPCUSTDATA;

typedef enum tagDESCKIND {
        DESCKIND_NONE=0,DESCKIND_FUNCDESC=DESCKIND_NONE+1,
        DESCKIND_VARDESC=DESCKIND_FUNCDESC+1,DESCKIND_TYPECOMP=DESCKIND_VARDESC+1,
        DESCKIND_IMPLICITAPPOBJ=DESCKIND_TYPECOMP+1,
        DESCKIND_MAX=DESCKIND_IMPLICITAPPOBJ+1
} DESCKIND;

typedef union tagBINDPTR {
        LPFUNCDESC lpfuncdesc;
        LPVARDESC lpvardesc;
        LPTYPECOMP lptcomp;
} BINDPTR,*LPBINDPTR;

#define INTERFACE IDispatch
DECLARE_INTERFACE_(IDispatch,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetTypeInfoCount)(THIS_ UINT*) PURE;
	STDMETHOD(GetTypeInfo)(THIS_ UINT,LCID,LPTYPEINFO*) PURE;
	STDMETHOD(GetIDsOfNames)(THIS_ REFIID,LPOLESTR*,UINT,LCID,DISPID*) PURE;
	STDMETHOD(Invoke)(THIS_ DISPID,REFIID,LCID,WORD,DISPPARAMS*,VARIANT*,EXCEPINFO*,UINT*) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define IDispatch_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IDispatch_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IDispatch_Release(p) (p)->lpVtbl->Release(p)
#define IDispatch_GetTypeInfoCount(p,a) (p)->lpVtbl->GetTypeInfoCount(p,a)
#define IDispatch_GetTypeInfo(p,a,b,c) (p)->lpVtbl->GetTypeInfo(p,a,b,c)
#define IDispatch_GetIDsOfNames(p,a,b,c,d,e) (p)->lpVtbl->GetIDsOfNames(p,a,b,c,d,e)
#define IDispatch_Invoke(p,a,b,c,d,e,f,g,h) (p)->lpVtbl->Invoke(p,a,b,c,d,e,f,g,h)
#endif

#define INTERFACE IEnumVARIANT
DECLARE_INTERFACE_(IEnumVARIANT,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Next)(THIS_ ULONG,VARIANT*,ULONG*) PURE;
	STDMETHOD(Skip)(THIS_ ULONG) PURE;
	STDMETHOD(Reset)(THIS) PURE;
	STDMETHOD(Clone)(THIS_ IEnumVARIANT**) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define IEnumVARIANT_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IEnumVARIANT_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IEnumVARIANT_Release(p) (p)->lpVtbl->Release(p)
#define IEnumVARIANT_Next(p,a,b,c) (p)->lpVtbl->Next(p,a,b,c)
#define IEnumVARIANT_Skip(p,a) (p)->lpVtbl->Skip(p,a)
#define IEnumVARIANT_Reset(p) (p)->lpVtbl->Reset(p)
#define IEnumVARIANT_Clone(p,a) (p)->lpVtbl->Clone(p,a)
#endif

#define INTERFACE ITypeComp
DECLARE_INTERFACE_(ITypeComp,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Bind)(THIS_ LPOLESTR,ULONG,WORD,LPTYPEINFO*,DESCKIND*,LPBINDPTR) PURE;
	STDMETHOD(BindType)(THIS_ LPOLESTR,ULONG,LPTYPEINFO*,LPTYPECOMP*) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ITypeComp_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ITypeComp_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITypeComp_Release(p) (p)->lpVtbl->Release(p)
#define ITypeComp_Bind(p,a,b,c,d,e,f) (p)->lpVtbl->Bind(p,a,b,c,d,e,f)
#define ITypeComp_BindType(p,a,b,c,d) (p)->lpVtbl->BindType(p,a,b,c,d)
#endif

#define INTERFACE ITypeInfo
DECLARE_INTERFACE_(ITypeInfo,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetTypeAttr)(THIS_ LPTYPEATTR*) PURE;
	STDMETHOD(GetTypeComp)(THIS_ LPTYPECOMP*) PURE;
	STDMETHOD(GetFuncDesc)(THIS_ UINT,LPFUNCDESC*) PURE;
	STDMETHOD(GetVarDesc)(THIS_ UINT,LPVARDESC*) PURE;
	STDMETHOD(GetNames)(THIS_ MEMBERID,BSTR*,UINT,UINT*) PURE;
	STDMETHOD(GetRefTypeOfImplType)(THIS_ UINT,HREFTYPE*) PURE;
	STDMETHOD(GetImplTypeFlags)(THIS_ UINT,INT*) PURE;
	STDMETHOD(GetIDsOfNames)(THIS_ LPOLESTR*,UINT,MEMBERID*) PURE;
	STDMETHOD(Invoke)(THIS_ PVOID,MEMBERID,WORD,DISPPARAMS*,VARIANT*,EXCEPINFO*,UINT*) PURE;
	STDMETHOD(GetDocumentation)(THIS_ MEMBERID,BSTR*,BSTR*,DWORD*,BSTR*) PURE;
	STDMETHOD(GetDllEntry)(THIS_ MEMBERID,INVOKEKIND,BSTR*,BSTR*,WORD*) PURE;
	STDMETHOD(GetRefTypeInfo)(THIS_ HREFTYPE,LPTYPEINFO*) PURE;
	STDMETHOD(AddressOfMember)(THIS_ MEMBERID,INVOKEKIND,PVOID*) PURE;
	STDMETHOD(CreateInstance)(THIS_ LPUNKNOWN,REFIID,PVOID*) PURE;
	STDMETHOD(GetMops)(THIS_ MEMBERID,BSTR*) PURE;
	STDMETHOD(GetContainingTypeLib)(THIS_ LPTYPELIB*,UINT*) PURE;
	STDMETHOD_(void,ReleaseTypeAttr)(THIS_ LPTYPEATTR) PURE;
	STDMETHOD_(void,ReleaseFuncDesc)(THIS_ LPFUNCDESC) PURE;
	STDMETHOD_(void,ReleaseVarDesc)(THIS_ LPVARDESC) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ITypeInfo_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ITypeInfo_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITypeInfo_Release(p) (p)->lpVtbl->Release(p)
#define ITypeInfo_GetTypeAttr(p,a) (p)->lpVtbl->GetTypeAttr(p,a)
#define ITypeInfo_GetTypeComp(p,a) (p)->lpVtbl->GetTypeComp(p,a)
#define ITypeInfo_GetFuncDesc(p,a,b) (p)->lpVtbl->GetFuncDesc(p,a,b)
#define ITypeInfo_GetVarDesc(p,a,b) (p)->lpVtbl->GetVarDesc(p,a,b)
#define ITypeInfo_GetNames(p,a,b,c,d) (p)->lpVtbl->GetNames(p,a,b,c,d)
#define ITypeInfo_GetRefTypeOfImplType(p,a,b) (p)->lpVtbl->GetRefTypeOfImplType(p,a,b)
#define ITypeInfo_GetImplTypeFlags(p,a,b) (p)->lpVtbl->GetImplTypeFlags(p,a,b)
#define ITypeInfo_GetIDsOfNames(p,a,b,c) (p)->lpVtbl->GetIDsOfNames(p,a,b,c)
#define ITypeInfo_Invoke(p,a,b,c,d,e,f,g) (p)->lpVtbl->Invoke(p,a,b,c,d,e,f,g)
#define ITypeInfo_GetDocumentation(p,a,b,c,d,e) (p)->lpVtbl->GetDocumentation(p,a,b,c,d,e)
#define ITypeInfo_GetDllEntry(p,a,b,c,d,e) (p)->lpVtbl->GetDllEntry(p,a,b,c,d,e)
#define ITypeInfo_GetRefTypeInfo(p,a,b) (p)->lpVtbl->GetRefTypeInfo(p,a,b)
#define ITypeInfo_AddressOfMember(p,a,b,c) (p)->lpVtbl->AddressOfMember(p,a,b,c)
#define ITypeInfo_CreateInstance(p,a,b,c) (p)->lpVtbl->CreateInstance(p,a,b,c)
#define ITypeInfo_GetMops(p,a,b) (p)->lpVtbl->GetMops(p,a,b)
#define ITypeInfo_GetContainingTypeLib(p,a,b) (p)->lpVtbl->GetContainingTypeLib(p,a,b)
#define ITypeInfo_ReleaseTypeAttr(p,a) (p)->lpVtbl->ReleaseTypeAttr(p,a)
#define ITypeInfo_ReleaseFuncDesc(p,a) (p)->lpVtbl->ReleaseFuncDesc(p,a)
#define ITypeInfo_ReleaseVarDesc(p,a) (p)->lpVtbl->ReleaseVarDesc(p,a)
#endif

#define INTERFACE ITypeInfo2
DECLARE_INTERFACE_(ITypeInfo2,ITypeInfo)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetTypeAttr)(THIS_ LPTYPEATTR*) PURE;
	STDMETHOD(GetTypeComp)(THIS_ LPTYPECOMP*) PURE;
	STDMETHOD(GetFuncDesc)(THIS_ UINT,LPFUNCDESC*) PURE;
	STDMETHOD(GetVarDesc)(THIS_ UINT,LPVARDESC*) PURE;
	STDMETHOD(GetNames)(THIS_ MEMBERID,BSTR*,UINT,UINT*) PURE;
	STDMETHOD(GetRefTypeOfImplType)(THIS_ UINT,HREFTYPE*) PURE;
	STDMETHOD(GetImplTypeFlags)(THIS_ UINT,INT*) PURE;
	STDMETHOD(GetIDsOfNames)(THIS_ LPOLESTR*,UINT,MEMBERID*) PURE;
	STDMETHOD(Invoke)(THIS_ PVOID,MEMBERID,WORD,DISPPARAMS*,VARIANT*,EXCEPINFO*,UINT*) PURE;
	STDMETHOD(GetDocumentation)(THIS_ MEMBERID,BSTR*,BSTR*,DWORD*,BSTR*) PURE;
	STDMETHOD(GetDllEntry)(THIS_ MEMBERID,INVOKEKIND,BSTR*,BSTR*,WORD*) PURE;
	STDMETHOD(GetRefTypeInfo)(THIS_ HREFTYPE,LPTYPEINFO*) PURE;
	STDMETHOD(AddressOfMember)(THIS_ MEMBERID,INVOKEKIND,PVOID*) PURE;
	STDMETHOD(CreateInstance)(THIS_ LPUNKNOWN,REFIID,PVOID*) PURE;
	STDMETHOD(GetMops)(THIS_ MEMBERID,BSTR*) PURE;
	STDMETHOD(GetContainingTypeLib)(THIS_ LPTYPELIB*,UINT*) PURE;
	STDMETHOD_(void,ReleaseTypeAttr)(THIS_ LPTYPEATTR) PURE;
	STDMETHOD_(void,ReleaseFuncDesc)(THIS_ LPFUNCDESC) PURE;
	STDMETHOD_(void,ReleaseVarDesc)(THIS_ LPVARDESC) PURE;
	STDMETHOD(GetTypeKind)(THIS_ TYPEKIND*) PURE;
	STDMETHOD(GetTypeFlags)(THIS_ ULONG*) PURE;
	STDMETHOD(GetFuncIndexOfMemId)(THIS_ MEMBERID,INVOKEKIND,UINT*) PURE;
	STDMETHOD(GetVarIndexOfMemId)(THIS_ MEMBERID,UINT*) PURE;
	STDMETHOD(GetCustData)(THIS_ REFGUID,VARIANT*) PURE;
	STDMETHOD(GetFuncCustData)(THIS_ UINT,REFGUID,VARIANT*) PURE;
	STDMETHOD(GetParamCustData)(THIS_ UINT,UINT,REFGUID,VARIANT*) PURE;
	STDMETHOD(GetVarCustData)(THIS_ UINT,REFGUID,VARIANT*) PURE;
	STDMETHOD(GetImplTypeCustData)(THIS_ UINT,REFGUID,VARIANT*) PURE;
	STDMETHOD(GetDocumentation2)(THIS_ MEMBERID,LCID,BSTR*,DWORD*,BSTR*) PURE;
	STDMETHOD(GetAllCustData)(THIS_ CUSTDATA*) PURE;
	STDMETHOD(GetAllFuncCustData)(THIS_ UINT,CUSTDATA*) PURE;
	STDMETHOD(GetAllParamCustData)(THIS_ UINT,UINT,CUSTDATA*) PURE;
	STDMETHOD(GetAllVarCustData)(THIS_ UINT,CUSTDATA*) PURE;
	STDMETHOD(GetAllImplTypeCustData)(THIS_ UINT,CUSTDATA*) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ITypeInfo2_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ITypeInfo2_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITypeInfo2_Release(p) (p)->lpVtbl->Release(p)
#define ITypeInfo2_GetTypeAttr(p,a) (p)->lpVtbl->GetTypeAttr(p,a)
#define ITypeInfo2_GetTypeComp(p,a) (p)->lpVtbl->GetTypeComp(p,a)
#define ITypeInfo2_GetFuncDesc(p,a,b) (p)->lpVtbl->GetFuncDesc(p,a,b)
#define ITypeInfo2_GetVarDesc(p,a,b) (p)->lpVtbl->GetVarDesc(p,a,b)
#define ITypeInfo2_GetNames(p,a,b,c,d) (p)->lpVtbl->GetNames(p,a,b,c,d)
#define ITypeInfo2_GetRefTypeOfImplType(p,a,b) (p)->lpVtbl->GetRefTypeOfImplType(p,a,b)
#define ITypeInfo2_GetImplTypeFlags(p,a,b) (p)->lpVtbl->GetImplTypeFlags(p,a,b)
#define ITypeInfo2_GetIDsOfNames(p,a,b,c) (p)->lpVtbl->GetIDsOfNames(p,a,b,c)
#define ITypeInfo2_Invoke(p,a,b,c,d,e,f,g) (p)->lpVtbl->Invoke(p,a,b,c,d,e,f,g)
#define ITypeInfo2_GetDocumentation(p,a,b,c,d,e) (p)->lpVtbl->GetDocumentation(p,a,b,c,d,e)
#define ITypeInfo2_GetDllEntry(p,a,b,c,d,e) (p)->lpVtbl->GetDllEntry(p,a,b,c,d,e)
#define ITypeInfo2_GetRefTypeInfo(p,a,b) (p)->lpVtbl->GetRefTypeInfo(p,a,b)
#define ITypeInfo2_AddressOfMember(p,a,b,c) (p)->lpVtbl->AddressOfMember(p,a,b,c)
#define ITypeInfo2_CreateInstance(p,a,b,c) (p)->lpVtbl->CreateInstance(p,a,b,c)
#define ITypeInfo2_GetMops(p,a,b) (p)->lpVtbl->GetMops(p,a,b)
#define ITypeInfo2_GetContainingTypeLib(p,a,b) (p)->lpVtbl->GetContainingTypeLib(p,a,b)
#define ITypeInfo2_ReleaseTypeAttr(p,a) (p)->lpVtbl->ReleaseTypeAttr(p,a)
#define ITypeInfo2_ReleaseFuncDesc(p,a) (p)->lpVtbl->ReleaseFuncDesc(p,a)
#define ITypeInfo2_ReleaseVarDesc(p,a) (p)->lpVtbl->ReleaseVarDesc(p,a)
#define ITypeInfo2_GetTypeKind(p,a) (p)->lpVtbl->GetTypeKind(p,a)
#define ITypeInfo2_GetTypeFlags(p,a) (p)->lpVtbl->GetTypeFlags(p,a)
#define ITypeInfo2_GetFuncIndexOfMemId(p,a,b,c) (p)->lpVtbl->GetFuncIndexOfMemId(p,a,b,c)
#define ITypeInfo2_GetVarIndexOfMemId(p,a,b) (p)->lpVtbl->GetVarIndexOfMemId(p,a,b)
#define ITypeInfo2_GetCustData(p,a,b) (p)->lpVtbl->GetCustData(p,a,b)
#define ITypeInfo2_GetFuncCustData(p,a,b,c) (p)->lpVtbl->GetFuncCustData(p,a,b,c)
#define ITypeInfo2_GetParamCustData(p,a,b,c,d) (p)->lpVtbl->GetParamCustData(p,a,b,c,d)
#define ITypeInfo2_GetVarCustData(p,a,b,c) (p)->lpVtbl->GetVarCustData(p,a,b,c)
#define ITypeInfo2_GetImplTypeCustData(p,a,b,c) (p)->lpVtbl->GetImplTypeCustData(p,a,b,c)
#define ITypeInfo2_GetDocumentation2(p,a,b,c,d,e) (p)->lpVtbl->GetDocumentation2(p,a,b,c,d,e)
#define ITypeInfo2_GetAllCustData(p,a) (p)->lpVtbl->GetAllCustData(p,a)
#define ITypeInfo2_GetAllFuncCustData(p,a,b) (p)->lpVtbl->GetAllFuncCustData(p,a,b)
#define ITypeInfo2_GetAllParamCustData(p,a,b,c) (p)->lpVtbl->GetAllParamCustData(p,a,b,c)
#define ITypeInfo2_GetAllVarCustData(p,a,b) (p)->lpVtbl->GetAllVarCustData(p,a,b)
#define ITypeInfo2_GetAllImplTypeCustData(p,a,b) (p)->lpVtbl->GetAllImplTypeCustData(p,a,b)
#endif

#define INTERFACE ITypeLib
DECLARE_INTERFACE_(ITypeLib,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD_(UINT,GetTypeInfoCount)(THIS) PURE;
	STDMETHOD(GetTypeInfo)(THIS_ UINT,ITypeInfo**) PURE;
	STDMETHOD(GetTypeInfoType)(THIS_ UINT,TYPEKIND*) PURE;
	STDMETHOD(GetTypeInfoOfGuid)(THIS_ REFGUID,ITypeInfo**) PURE;
	STDMETHOD(GetLibAttr)(THIS_ TLIBATTR**) PURE;
	STDMETHOD(GetTypeComp)(THIS_ LPTYPECOMP*) PURE;
	STDMETHOD(GetDocumentation)(THIS_ INT,BSTR*,BSTR*,DWORD*,BSTR*) PURE;
	STDMETHOD(IsName)(THIS_ LPOLESTR,ULONG,BOOL*) PURE;
	STDMETHOD(FindName)(THIS_ LPOLESTR,ULONG,ITypeInfo**,MEMBERID*,USHORT*) PURE;
	STDMETHOD_(void,ReleaseTLibAttr)(THIS_ TLIBATTR*) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ITypeLib_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ITypeLib_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITypeLib_Release(p) (p)->lpVtbl->Release(p)
#define ITypeLib_GetTypeInfoCount(p) (p)->lpVtbl->GetTypeInfoCount(p)
#define ITypeLib_GetTypeInfo(p,a,b) (p)->lpVtbl->GetTypeInfo(p,a,b)
#define ITypeLib_GetTypeInfoType(p,a,b) (p)->lpVtbl->GetTypeInfoType(p,a,b)
#define ITypeLib_GetTypeInfoOfGuid(p,a,b) (p)->lpVtbl->GetTypeInfoOfGuid(p,a,b)
#define ITypeLib_GetLibAttr(p,a) (p)->lpVtbl->GetLibAttr(p,a)
#define ITypeLib_GetTypeComp(p,a) (p)->lpVtbl->GetTypeComp(p,a)
#define ITypeLib_GetDocumentation(p,a,b,c,d,e) (p)->lpVtbl->GetDocumentation(p,a,b,c,d,e)
#define ITypeLib_IsName(p,a,b,c) (p)->lpVtbl->IsName(p,a,b,c)
#define ITypeLib_FindName(p,a,b,c,d,e) (p)->lpVtbl->FindName(p,a,b,c,d,e)
#define ITypeLib_ReleaseTLibAttr(p,a) (p)->lpVtbl->ReleaseTLibAttr(p,a)
#endif

#define INTERFACE ITypeLib2
DECLARE_INTERFACE_(ITypeLib2,ITypeLib)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD_(UINT,GetTypeInfoCount)(THIS) PURE;
	STDMETHOD(GetTypeInfo)(THIS_ UINT,ITypeInfo**) PURE;
	STDMETHOD(GetTypeInfoType)(THIS_ UINT,TYPEKIND*) PURE;
	STDMETHOD(GetTypeInfoOfGuid)(THIS_ REFGUID,ITypeInfo**) PURE;
	STDMETHOD(GetLibAttr)(THIS_ TLIBATTR**) PURE;
	STDMETHOD(GetTypeComp)(THIS_ LPTYPECOMP*) PURE;
	STDMETHOD(GetDocumentation)(THIS_ INT,BSTR*,BSTR*,DWORD*,BSTR*) PURE;
	STDMETHOD(IsName)(THIS_ LPOLESTR,ULONG,BOOL*) PURE;
	STDMETHOD(FindName)(THIS_ LPOLESTR,ULONG,ITypeInfo**,MEMBERID*,USHORT*) PURE;
	STDMETHOD_(void,ReleaseTLibAttr)(THIS_ TLIBATTR*) PURE;
	STDMETHOD(GetCustData)(THIS_ REFGUID,VARIANT*) PURE;
	STDMETHOD(GetLibStatistics)(THIS_ ULONG*,ULONG*) PURE;
	STDMETHOD(GetDocumentation2)(THIS_ INT,LCID,BSTR*,DWORD*,BSTR*) PURE;
	STDMETHOD(GetAllCustData)(THIS_ CUSTDATA*) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ITypeLib2_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ITypeLib2_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITypeLib2_Release(p) (p)->lpVtbl->Release(p)
#define ITypeLib2_GetTypeInfoCount(p) (p)->lpVtbl->GetTypeInfoCount(p)
#define ITypeLib2_GetTypeInfo(p,a,b) (p)->lpVtbl->GetTypeInfo(p,a,b)
#define ITypeLib2_GetTypeInfoType(p,a,b) (p)->lpVtbl->GetTypeInfoType(p,a,b)
#define ITypeLib2_GetTypeInfoOfGuid(p,a,b) (p)->lpVtbl->GetTypeInfoOfGuid(p,a,b)
#define ITypeLib2_GetLibAttr(p,a) (p)->lpVtbl->GetLibAttr(p,a)
#define ITypeLib2_GetTypeComp(p,a) (p)->lpVtbl->GetTypeComp(p,a)
#define ITypeLib2_GetDocumentation(p,a,b,c,d,e) (p)->lpVtbl->GetDocumentation(p,a,b,c,d,e)
#define ITypeLib2_IsName(p,a,b,c) (p)->lpVtbl->IsName(p,a,b,c)
#define ITypeLib2_FindName(p,a,b,c,d,e) (p)->lpVtbl->FindName(p,a,b,c,d,e)
#define ITypeLib2_ReleaseTLibAttr(p,a) (p)->lpVtbl->ReleaseTLibAttr(p,a)
#define ITypeLib2_GetCustData(p,a,b) (p)->lpVtbl->GetCustData(p,a,b)
#define ITypeLib2_GetLibStatistics(p,a,b) (p)->lpVtbl->GetLibStatistics(p,a,b)
#define ITypeLib2_GetDocumentation2(p,a,b,c,d,e) (p)->lpVtbl->GetDocumentation2(p,a,b,c,d,e)
#define ITypeLib2_GetAllCustData(p,a) (p)->lpVtbl->GetAllCustData(p,a)
#endif

#define INTERFACE ITypeFactory
DECLARE_INTERFACE_(ITypeFactory,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetAllCustData)(THIS_ CUSTDATA*) PURE;
    STDMETHOD(CreateFromTypeInfo)(THIS_ LPTYPEINFO,REFIID,LPUNKNOWN*);
};
#undef INTERFACE

#ifdef COBJMACROS
#define ITypeFactory_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ITypeFactory_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ITypeFactory_Release(p) (p)->lpVtbl->Release(p)
#define ITypeFactory_CreateFromTypeInfo(p,a,b,c) (p)->lpVtbl->CreateFromTypeInfo(p,a,b,c)
#endif

#define INTERFACE ICreateTypeInfo
DECLARE_INTERFACE_(ICreateTypeInfo,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(SetGuid)(THIS_ REFGUID) PURE;
	STDMETHOD(SetTypeFlags)(THIS_ UINT) PURE;
	STDMETHOD(SetDocString)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpContext)(THIS_ DWORD) PURE;
	STDMETHOD(SetVersion)(THIS_ WORD,WORD) PURE;
	STDMETHOD(AddRefTypeInfo)(THIS_ ITypeInfo*,HREFTYPE*) PURE;
	STDMETHOD(AddFuncDesc)(THIS_ UINT,FUNCDESC*) PURE;
	STDMETHOD(AddImplType)(THIS_ UINT,HREFTYPE) PURE;
	STDMETHOD(SetImplTypeFlags)(THIS_ UINT,INT) PURE;
	STDMETHOD(SetAlignment)(THIS_ WORD) PURE;
	STDMETHOD(SetSchema)(THIS_ LPOLESTR) PURE;
	STDMETHOD(AddVarDesc)(THIS_ UINT,VARDESC*) PURE;
	STDMETHOD(SetFuncAndParamNames)(THIS_ UINT,LPOLESTR*,UINT) PURE;
	STDMETHOD(SetVarName)(THIS_ UINT,LPOLESTR) PURE;
	STDMETHOD(SetTypeDescAlias)(THIS_ TYPEDESC*) PURE;
	STDMETHOD(DefineFuncAsDllEntry)(THIS_ UINT,LPOLESTR,LPOLESTR) PURE;
	STDMETHOD(SetFuncDocString)(THIS_ UINT,LPOLESTR) PURE;
	STDMETHOD(SetVarDocString)(THIS_ UINT,LPOLESTR) PURE;
	STDMETHOD(SetFuncHelpContext)(THIS_ UINT,DWORD) PURE;
	STDMETHOD(SetVarHelpContext)(THIS_ UINT,DWORD) PURE;
	STDMETHOD(SetMops)(THIS_ UINT,BSTR) PURE;
	STDMETHOD(SetTypeIdldesc)(THIS_ IDLDESC*) PURE;
	STDMETHOD(LayOut)(THIS) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ICreateTypeInfo_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ICreateTypeInfo_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ICreateTypeInfo_Release(p) (p)->lpVtbl->Release(p)
#define ICreateTypeInfo_SetGuid(p,a) (p)->lpVtbl->SetGuid(p,a)
#define ICreateTypeInfo_SetTypeFlags(p,a) (p)->lpVtbl->SetTypeFlags(p,a)
#define ICreateTypeInfo_SetDocString(p,a) (p)->lpVtbl->SetDocString(p,a)
#define ICreateTypeInfo_SetHelpContext(p,a) (p)->lpVtbl->SetHelpContext(p,a)
#define ICreateTypeInfo_SetVersion(p,a,b) (p)->lpVtbl->SetVersion(p,a,b)
#define ICreateTypeInfo_AddRefTypeInfo(p,a,b) (p)->lpVtbl->AddRefTypeInfo(p,a,b)
#define ICreateTypeInfo_AddFuncDesc(p,a,b) (p)->lpVtbl->AddFuncDesc(p,a,b)
#define ICreateTypeInfo_AddImplType(p,a,b) (p)->lpVtbl->AddImplType(p,a,b)
#define ICreateTypeInfo_SetImplTypeFlags(p,a,b) (p)->lpVtbl->SetImplTypeFlags(p,a,b)
#define ICreateTypeInfo_SetAlignment(p,a) (p)->lpVtbl->SetAlignment(p,a)
#define ICreateTypeInfo_SetSchema(p,a) (p)->lpVtbl->SetSchema(p,a)
#define ICreateTypeInfo_AddVarDesc(p,a,b) (p)->lpVtbl->AddVarDesc(p,a,b)
#define ICreateTypeInfo_SetFuncAndParamNames(p,a,b,c) (p)->lpVtbl->SetFuncAndParamNames(p,a,b,c)
#define ICreateTypeInfo_SetVarName(p,a,b) (p)->lpVtbl->SetVarName(p,a,b)
#define ICreateTypeInfo_SetTypeDescAlias(p,a) (p)->lpVtbl->SetTypeDescAlias(p,a)
#define ICreateTypeInfo_DefineFuncAsDllEntry(p,a,b,c) (p)->lpVtbl->DefineFuncAsDllEntry(p,a,b,c)
#define ICreateTypeInfo_SetFuncDocString(p,a,b) (p)->lpVtbl->SetFuncDocString(p,a,b)
#define ICreateTypeInfo_SetVarDocString(p,a,b) (p)->lpVtbl->SetVarDocString(p,a,b)
#define ICreateTypeInfo_SetFuncHelpContext(p,a,b) (p)->lpVtbl->SetFuncHelpContext(p,a,b)
#define ICreateTypeInfo_SetVarHelpContext(p,a,b) (p)->lpVtbl->SetVarHelpContext(p,a,b)
#define ICreateTypeInfo_SetMops(p,a,b) (p)->lpVtbl->SetMops(p,a,b)
#define ICreateTypeInfo_SetTypeIdldesc(p,a) (p)->lpVtbl->SetTypeIdldesc(p,a)
#define ICreateTypeInfo_LayOut(p) (p)->lpVtbl->LayOut(p)
#endif

#define INTERFACE ICreateTypeInfo2
DECLARE_INTERFACE_(ICreateTypeInfo2,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(SetGuid)(THIS_ REFGUID) PURE;
	STDMETHOD(SetTypeFlags)(THIS_ UINT) PURE;
	STDMETHOD(SetDocString)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpContext)(THIS_ DWORD) PURE;
	STDMETHOD(SetVersion)(THIS_ WORD,WORD) PURE;
	STDMETHOD(AddRefTypeInfo)(THIS_ ITypeInfo*,HREFTYPE*) PURE;
	STDMETHOD(AddFuncDesc)(THIS_ UINT,FUNCDESC*) PURE;
	STDMETHOD(AddImplType)(THIS_ UINT,HREFTYPE) PURE;
	STDMETHOD(SetImplTypeFlags)(THIS_ UINT,INT) PURE;
	STDMETHOD(SetAlignment)(THIS_ WORD) PURE;
	STDMETHOD(SetSchema)(THIS_ LPOLESTR) PURE;
	STDMETHOD(AddVarDesc)(THIS_ UINT,VARDESC*) PURE;
	STDMETHOD(SetFuncAndParamNames)(THIS_ UINT,LPOLESTR*,UINT) PURE;
	STDMETHOD(SetVarName)(THIS_ UINT,LPOLESTR) PURE;
	STDMETHOD(SetTypeDescAlias)(THIS_ TYPEDESC*) PURE;
	STDMETHOD(DefineFuncAsDllEntry)(THIS_ UINT,LPOLESTR,LPOLESTR) PURE;
	STDMETHOD(SetFuncDocString)(THIS_ UINT,LPOLESTR) PURE;
	STDMETHOD(SetVarDocString)(THIS_ UINT,LPOLESTR) PURE;
	STDMETHOD(SetFuncHelpContext)(THIS_ UINT,DWORD) PURE;
	STDMETHOD(SetVarHelpContext)(THIS_ UINT,DWORD) PURE;
	STDMETHOD(SetMops)(THIS_ UINT,BSTR) PURE;
	STDMETHOD(SetTypeIdldesc)(THIS_ IDLDESC*) PURE;
	STDMETHOD(LayOut)(THIS) PURE;
	STDMETHOD(DeleteFuncDesc)(THIS_ UINT) PURE;
	STDMETHOD(DeleteFuncDescByMemId)(THIS_ MEMBERID, INVOKEKIND) PURE;
	STDMETHOD(DeleteVarDesc)(THIS_ UINT) PURE;
	STDMETHOD(DeleteVarDescByMemId)(THIS_ MEMBERID) PURE;        
	STDMETHOD(DeleteImplType)(THIS_ UINT) PURE;        
	STDMETHOD(SetCustData)(THIS_ REFGUID, VARIANT*) PURE;        
	STDMETHOD(SetFuncCustData)(THIS_ UINT, REFGUID, VARIANT*) PURE;        
	STDMETHOD(SetParamCustData)(THIS_ UINT, UINT, REFGUID, VARIANT*) PURE;        
	STDMETHOD(SetVarCustData)(THIS_ UINT, REFGUID, VARIANT*) PURE;        
	STDMETHOD(SetImplTypeCustData)(THIS_ UINT, REFGUID, VARIANT*) PURE;        
	STDMETHOD(SetHelpStringContext)(THIS_ ULONG) PURE;        
	STDMETHOD(SetFuncHelpStringContext)(THIS_ UINT, ULONG) PURE;        
	STDMETHOD(SetVarHelpStringContext)(THIS_ UINT, ULONG) PURE;        
	STDMETHOD(Invalidate)(THIS) PURE;       
	STDMETHOD(SetName)(THIS_ LPOLESTR) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ICreateTypeInfo2_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ICreateTypeInfo2_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ICreateTypeInfo2_Release(p) (p)->lpVtbl->Release(p)
#define ICreateTypeInfo2_SetGuid(p,a) (p)->lpVtbl->SetGuid(p,a)
#define ICreateTypeInfo2_SetTypeFlags(p,a) (p)->lpVtbl->SetTypeFlags(p,a)
#define ICreateTypeInfo2_SetDocString(p,a) (p)->lpVtbl->SetDocString(p,a)
#define ICreateTypeInfo2_SetHelpContext(p,a) (p)->lpVtbl->SetHelpContext(p,a)
#define ICreateTypeInfo2_SetVersion(p,a,b) (p)->lpVtbl->SetVersion(p,a,b)
#define ICreateTypeInfo2_AddRefTypeInfo(p,a,b) (p)->lpVtbl->AddRefTypeInfo(p,a,b)
#define ICreateTypeInfo2_AddFuncDesc(p,a,b) (p)->lpVtbl->AddFuncDesc(p,a,b)
#define ICreateTypeInfo2_AddImplType(p,a,b) (p)->lpVtbl->AddImplType(p,a,b)
#define ICreateTypeInfo2_SetImplTypeFlags(p,a,b) (p)->lpVtbl->SetImplTypeFlags(p,a,b)
#define ICreateTypeInfo2_SetAlignment(p,a) (p)->lpVtbl->SetAlignment(p,a)
#define ICreateTypeInfo2_SetSchema(p,a) (p)->lpVtbl->SetSchema(p,a)
#define ICreateTypeInfo2_AddVarDesc(p,a,b) (p)->lpVtbl->AddVarDesc(p,a,b)
#define ICreateTypeInfo2_SetFuncAndParamNames(p,a,b,c) (p)->lpVtbl->SetFuncAndParamNames(p,a,b,c)
#define ICreateTypeInfo2_SetVarName(p,a,b) (p)->lpVtbl->SetVarName(p,a,b)
#define ICreateTypeInfo2_SetTypeDescAlias(p,a) (p)->lpVtbl->SetTypeDescAlias(p,a)
#define ICreateTypeInfo2_DefineFuncAsDllEntry(p,a,b,c) (p)->lpVtbl->DefineFuncAsDllEntry(p,a,b,c)
#define ICreateTypeInfo2_SetFuncDocString(p,a,b) (p)->lpVtbl->SetFuncDocString(p,a,b)
#define ICreateTypeInfo2_SetVarDocString(p,a,b) (p)->lpVtbl->SetVarDocString(p,a,b)
#define ICreateTypeInfo2_SetFuncHelpContext(p,a,b) (p)->lpVtbl->SetFuncHelpContext(p,a,b)
#define ICreateTypeInfo2_SetVarHelpContext(p,a,b) (p)->lpVtbl->SetVarHelpContext(p,a,b)
#define ICreateTypeInfo2_SetMops(p,a,b) (p)->lpVtbl->SetMops(p,a,b)
#define ICreateTypeInfo2_SetTypeIdldesc(p,a) (p)->lpVtbl->SetTypeIdldesc(p,a)
#define ICreateTypeInfo2_LayOut(p) (p)->lpVtbl->LayOut(p)
#define ICreateTypeInfo2_DeleteFuncDesc(p,a) (p)->lpVtbl->DeleteFuncDesc(p,a)
#define ICreateTypeInfo2_DeleteFuncDescByMemId(p,a,b) (p)->lpVtbl->DeleteFuncDescByMemId(p,a,b)
#define ICreateTypeInfo2_DeleteVarDesc(p,a) (p)->lpVtbl->DeleteVarDesc(p,a)
#define ICreateTypeInfo2_DeleteVarDescByMemId(p,a) (p)->lpVtbl->DeleteVarDescByMemId(p,a)
#define ICreateTypeInfo2_DeleteImplType(p,a) (p)->lpVtbl->DeleteImplType(p,a)
#define ICreateTypeInfo2_SetCustData(p,a,b) (p)->lpVtbl->SetCustData(p,a,b)
#define ICreateTypeInfo2_SetFuncCustData(p,a,b,c) (p)->lpVtbl->SetFuncCustData(p,a,b,c)
#define ICreateTypeInfo2_SetParamCustData(p,a,b,c,d) (p)->lpVtbl->SetParamCustData(p,a,b,c,d)
#define ICreateTypeInfo2_SetVarCustData(p,a,b,c) (p)->lpVtbl->SetVarCustData(p,a,b,c)
#define ICreateTypeInfo2_SetImplTypeCustData(p,a,b,c) (p)->lpVtbl->SetImplTypeCustData(p,a,b,c)
#define ICreateTypeInfo2_SetHelpStringContext(p,a) (p)->lpVtbl->SetHelpStringContext(p,a)
#define ICreateTypeInfo2_SetFuncHelpStringContext(p,a,b) (p)->lpVtbl->SetFuncHelpStringContext(p,a,b)
#define ICreateTypeInfo2_SetVarHelpStringContext(p,a,b) (p)->lpVtbl->SetVarHelpStringContext(p,a,b)
#define ICreateTypeInfo2_Invalidate(p) (p)->lpVtbl->Invalidate(p)
#define ICreateTypeInfo2_SetName(p,a) (p)->lpVtbl->SetName(p,a)
#endif

#define INTERFACE ICreateTypeLib
DECLARE_INTERFACE_(ICreateTypeLib,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(CreateTypeInfo)(THIS_ LPOLESTR,TYPEKIND,ICreateTypeInfo**) PURE;
	STDMETHOD(SetName)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetVersion)(THIS_ WORD,WORD) PURE;
	STDMETHOD(SetGuid)(THIS_ REFGUID) PURE;
	STDMETHOD(SetDocString)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpFileName)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpContext)(THIS_ DWORD) PURE;
	STDMETHOD(SetLcid)(THIS_ LCID) PURE;
	STDMETHOD(SetLibFlags)(THIS_ UINT) PURE;
	STDMETHOD(SaveAllChanges)(THIS) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ICreateTypeLib_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ICreateTypeLib_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ICreateTypeLib_Release(p) (p)->lpVtbl->Release(p)
#define ICreateTypeLib_CreateTypeInfo(p,a,b,c) (p)->lpVtbl->CreateTypeInfo(p,a,b,c)
#define ICreateTypeLib_SetName(p,a) (p)->lpVtbl->SetName(p,a)
#define ICreateTypeLib_SetVersion(p,a,b) (p)->lpVtbl->SetVersion(p,a,b)
#define ICreateTypeLib_SetGuid(p,a) (p)->lpVtbl->SetGuid(p,a)
#define ICreateTypeLib_SetDocString(p,a) (p)->lpVtbl->SetDocString(p,a)
#define ICreateTypeLib_SetHelpFileName(p,a) (p)->lpVtbl->SetHelpFileName(p,a)
#define ICreateTypeLib_SetHelpContext(p,a) (p)->lpVtbl->SetHelpContext(p,a)
#define ICreateTypeLib_SetLcid(p,a) (p)->lpVtbl->SetLcid(p,a)
#define ICreateTypeLib_SetLibFlags(p,a) (p)->lpVtbl->SetLibFlags(p,a)
#define ICreateTypeLib_SaveAllChanges(p) (p)->lpVtbl->SaveAllChanges(p)
#endif

#define INTERFACE ICreateTypeLib2
DECLARE_INTERFACE_(ICreateTypeLib2,IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(CreateTypeInfo)(THIS_ LPOLESTR,TYPEKIND,ICreateTypeInfo**) PURE;
	STDMETHOD(SetName)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetVersion)(THIS_ WORD,WORD) PURE;
	STDMETHOD(SetGuid)(THIS_ REFGUID) PURE;
	STDMETHOD(SetDocString)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpFileName)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpContext)(THIS_ DWORD) PURE;
	STDMETHOD(SetLcid)(THIS_ LCID) PURE;
	STDMETHOD(SetLibFlags)(THIS_ UINT) PURE;
	STDMETHOD(SaveAllChanges)(THIS) PURE;
	STDMETHOD(DeleteTypeInfo)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetCustData)(THIS_ REFGUID, VARIANT*) PURE;
	STDMETHOD(SetHelpStringContext)(THIS_ ULONG) PURE;        
	STDMETHOD(SetHelpStringDll)(THIS_ LPOLESTR) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define ICreateTypeLib2_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ICreateTypeLib2_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ICreateTypeLib2_Release(p) (p)->lpVtbl->Release(p)
#define ICreateTypeLib2_CreateTypeInfo(p,a,b,c) (p)->lpVtbl->CreateTypeInfo(p,a,b,c)
#define ICreateTypeLib2_SetName(p,a) (p)->lpVtbl->SetName(p,a)
#define ICreateTypeLib2_SetVersion(p,a,b) (p)->lpVtbl->SetVersion(p,a,b)
#define ICreateTypeLib2_SetGuid(p,a) (p)->lpVtbl->SetGuid(p,a)
#define ICreateTypeLib2_SetDocString(p,a) (p)->lpVtbl->SetDocString(p,a)
#define ICreateTypeLib2_SetHelpFileName(p,a) (p)->lpVtbl->SetHelpFileName(p,a)
#define ICreateTypeLib2_SetHelpContext(p,a) (p)->lpVtbl->SetHelpContext(p,a)
#define ICreateTypeLib2_SetLcid(p,a) (p)->lpVtbl->SetLcid(p,a)
#define ICreateTypeLib2_SetLibFlags(p,a) (p)->lpVtbl->SetLibFlags(p,a)
#define ICreateTypeLib2_SaveAllChanges(p) (p)->lpVtbl->SaveAllChanges(p)
#define ICreateTypeLib2_DeleteTypeInfo(p,a) (p)->lpVtbl->DeleteTypeInfo(p,a)
#define ICreateTypeLib2_SetCustData(p,a,b) (p)->lpVtbl->SetCustData(p,a,b)
#define ICreateTypeLib2_SetHelpStringContext(p,a) (p)->lpVtbl->SetHelpStringContext(p,a)
#define ICreateTypeLib2_SetHelpStringDll(p,a) (p)->lpVtbl->SetHelpStringDll(p,a)
#endif

EXTERN_C const IID IID_IErrorInfo;
#define INTERFACE IErrorInfo
DECLARE_INTERFACE_(IErrorInfo, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetGUID)(THIS_ GUID*) PURE;
	STDMETHOD(GetSource)(THIS_ BSTR*) PURE;
	STDMETHOD(GetDescription)(THIS_ BSTR*) PURE;
	STDMETHOD(GetHelpFile)(THIS_ BSTR*) PURE;
	STDMETHOD(GetHelpContext)(THIS_ DWORD*) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define IErrorInfo_QueryInterface(T,a,b) (T)->lpVtbl->QueryInterface(T,a,b)
#define IErrorInfo_AddRef(T) (T)->lpVtbl->AddRef(T)
#define IErrorInfo_Release(T) (T)->lpVtbl->Release(T)
#define IErrorInfo_GetGUID(T,a) (T)->lpVtbl->GetGUID(T,a)
#define IErrorInfo_GetSource(T,a) (T)->lpVtbl->GetSource(T,a)
#define IErrorInfo_GetDescription(T,a) (T)->lpVtbl->GetDescription(T,a)
#define IErrorInfo_GetHelpFile(T,a) (T)->lpVtbl->GetHelpFile(T,a)
#define IErrorInfo_GetHelpContext(T,a) (T)->lpVtbl->GetHelpContext(T,a)
#endif

EXTERN_C const IID IID_ICreateErrorInfo;
#define INTERFACE ICreateErrorInfo
DECLARE_INTERFACE_(ICreateErrorInfo, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*)PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(SetGUID)(THIS_ REFGUID) PURE;
	STDMETHOD(SetSource)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetDescription)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpFile)(THIS_ LPOLESTR) PURE;
	STDMETHOD(SetHelpContext)(THIS_ DWORD) PURE;
};
#undef INTERFACE

EXTERN_C const IID IID_ISupportErrorInfo;
#define INTERFACE ISupportErrorInfo
DECLARE_INTERFACE_(ISupportErrorInfo, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(InterfaceSupportsErrorInfo)(THIS_ REFIID) PURE;
};
#undef INTERFACE

EXTERN_C const IID IID_IRecordInfo;
#define INTERFACE IRecordInfo
DECLARE_INTERFACE_(IRecordInfo, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(RecordInit)(THIS_ PVOID) PURE;
	STDMETHOD(RecordClear)(THIS_ PVOID) PURE;
	STDMETHOD(RecordCopy)(THIS_ PVOID, PVOID) PURE;
	STDMETHOD(GetGuid)(THIS_ GUID*) PURE;
	STDMETHOD(GetName)(THIS_ BSTR*) PURE;
	STDMETHOD(GetSize)(THIS_ ULONG*) PURE;
	STDMETHOD(GetTypeInfo)(THIS_ ITypeInfo**) PURE;
	STDMETHOD(GetField)(THIS_ PVOID,LPCOLESTR,VARIANT*) PURE;
	STDMETHOD(GetFieldNoCopy)(THIS_ PVOID,LPCOLESTR,VARIANT*,PVOID*) PURE;
	STDMETHOD(PutField )(THIS_ ULONG,PVOID,LPCOLESTR, VARIANT*) PURE;
	STDMETHOD(PutFieldNoCopy)(THIS_ ULONG,PVOID,LPCOLESTR,VARIANT*) PURE;
	STDMETHOD(GetFieldNames)(THIS_ ULONG*,BSTR*) PURE;
	STDMETHOD_(BOOL,IsMatchingType)(THIS_ THIS) PURE;
	STDMETHOD_(PVOID,RecordCreate)(THIS) PURE;
	STDMETHOD(RecordCreateCopy)(THIS_ PVOID,PVOID*) PURE;
	STDMETHOD(RecordDestroy )(THIS_ PVOID) PURE;
};
#undef INTERFACE

#ifdef COBJMACROS
#define IRecordInfo_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IRecordInfo_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IRecordInfo_Release(p) (p)->lpVtbl->Release(p)
#define IRecordInfo_RecordInit(p,a) (p)->lpVtbl->RecordInit(p,a)
#define IRecordInfo_RecordClear(p,a) (p)->lpVtbl->RecordClear(p,a)
#define IRecordInfo_RecordCopy(p,a,b) (p)->lpVtbl->RecordCopy(p,a,b)
#define IRecordInfo_GetGuid(p,a) (p)->lpVtbl->GetGuid(p,a)
#define IRecordInfo_GetName(p,a) (p)->lpVtbl->GetName(p,a)
#define IRecordInfo_GetSize(p,a) (p)->lpVtbl->GetSize(p,a)
#define IRecordInfo_GetTypeInfo(p,a) (p)->lpVtbl->GetTypeInfo(p,a)
#define IRecordInfo_GetField(p,a,b,c) (p)->lpVtbl->GetField(p,a,b,c)
#define IRecordInfo_GetFieldNoCopy(p,a,b,c,d) (p)->lpVtbl->GetFieldNoCopy(p,a,b,c,d)
#define IRecordInfo_PutField(p,a,b,c,d) (p)->lpVtbl->PutField(p,a,b,c,d)
#define IRecordInfo_PutFieldNoCopy(p,a,b,c,d) (p)->lpVtbl->PutFieldNoCopy(p,a,b,c,d)
#define IRecordInfo_GetFieldNames(p,a,b) (p)->lpVtbl->GetFieldNames(p,a,b)
#define IRecordInfo_IsMatchingType(p,a) (p)->lpVtbl->IsMatchingType(p,a)
#define IRecordInfo_RecordCreate(p) (p)->lpVtbl->RecordCreate(p)
#define IRecordInfo_RecordCreateCopy(p,a,b) (p)->lpVtbl->RecordCreateCopy(p,a,b)
#define IRecordInfo_RecordDestroy(p,a) (p)->lpVtbl->RecordDestroy(p,a)
#endif

EXTERN_C const IID IID_ITypeMarshal;
#define INTERFACE ITypeMarshal
DECLARE_INTERFACE_(ITypeMarshal, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Size)(THIS_ PVOID,DWORD,PVOID,ULONG*) PURE;
	STDMETHOD(Marshal)(THIS_ PVOID,DWORD,PVOID,ULONG,BYTE*,ULONG*) PURE;
	STDMETHOD(Unmarshal)(THIS_ PVOID,DWORD,ULONG,BYTE*,ULONG*) PURE;
	STDMETHOD(Free)(THIS_ PVOID) PURE;
};
#undef INTERFACE

unsigned long __RPC_USER BSTR_UserSize(unsigned long*,unsigned long,LPBSTR);
unsigned char* __RPC_USER BSTR_UserMarshal(unsigned long*,unsigned char*,LPBSTR);
unsigned char* __RPC_USER BSTR_UserUnmarshal(unsigned long*,unsigned char*,LPBSTR);
void __RPC_USER BSTR_UserFree(unsigned long*,LPBSTR);
unsigned long __RPC_USER CLEANLOCALSTORAGE_UserSize(unsigned long*,unsigned long,CLEANLOCALSTORAGE*);
unsigned char* __RPC_USER CLEANLOCALSTORAGE_UserMarshal(unsigned long*,unsigned char*,CLEANLOCALSTORAGE*);
unsigned char* __RPC_USER CLEANLOCALSTORAGE_UserUnmarshal(unsigned long*,unsigned char*,CLEANLOCALSTORAGE*);
void __RPC_USER CLEANLOCALSTORAGE_UserFree(unsigned long*,CLEANLOCALSTORAGE*);
unsigned long __RPC_USER VARIANT_UserSize(unsigned long*,unsigned long,LPVARIANT);
unsigned char* __RPC_USER VARIANT_UserMarshal(unsigned long*,unsigned char*,LPVARIANT);
unsigned char* __RPC_USER VARIANT_UserUnmarshal(unsigned long*,unsigned char*,LPVARIANT);
void __RPC_USER VARIANT_UserFree(unsigned long*,LPVARIANT);
HRESULT STDMETHODCALLTYPE IDispatch_Invoke_Proxy(LPDISPATCH,DISPID,REFIID,LCID,WORD,DISPPARAMS*,LPVARIANT,LPEXCEPINFO,LPUINT);
HRESULT STDMETHODCALLTYPE IDispatch_Invoke_Stub(LPDISPATCH,DISPID,REFIID,LCID,DWORD,DISPPARAMS*,LPVARIANT,LPEXCEPINFO,LPUINT,UINT,LPUINT,LPVARIANTARG);
HRESULT STDMETHODCALLTYPE IDispatch_RemoteInvoke_Proxy(LPDISPATCH,DISPID,REFIID,LCID,DWORD,DISPPARAMS*,LPVARIANT,LPEXCEPINFO,LPUINT,UINT,LPUINT,LPVARIANTARG);
HRESULT STDMETHODCALLTYPE IEnumVARIANT_Next_Proxy(LPENUMVARIANT,ULONG,LPVARIANT,PULONG);
HRESULT STDMETHODCALLTYPE IEnumVARIANT_Next_Stub(LPENUMVARIANT,ULONG,LPVARIANT,PULONG);
HRESULT STDMETHODCALLTYPE IEnumVARIANT_RemoteNext_Proxy(LPENUMVARIANT,ULONG,LPVARIANT,PULONG);
HRESULT STDMETHODCALLTYPE ITypeComp_Bind_Proxy(LPTYPECOMP,LPOLESTR,ULONG,WORD,LPTYPEINFO*,DESCKIND*,BINDPTR*);
HRESULT STDMETHODCALLTYPE ITypeComp_Bind_Stub(LPTYPECOMP,LPOLESTR,ULONG,WORD,LPTYPEINFO*,DESCKIND*,LPFUNCDESC*,LPVARDESC*,LPTYPECOMP*,CLEANLOCALSTORAGE*);
HRESULT STDMETHODCALLTYPE ITypeComp_BindType_Proxy(LPTYPECOMP,LPOLESTR,ULONG,LPTYPEINFO*,LPTYPECOMP*);
HRESULT STDMETHODCALLTYPE ITypeComp_BindType_Stub(LPTYPECOMP,LPOLESTR,ULONG,LPTYPEINFO*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetTypeAttr_Proxy(LPTYPEINFO,LPTYPEATTR*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetTypeAttr_Stub(LPTYPEINFO,LPTYPEATTR*,CLEANLOCALSTORAGE*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetFuncDesc_Proxy(LPTYPEINFO,UINT,FUNCDESC**);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetFuncDesc_Stub(LPTYPEINFO,UINT,LPFUNCDESC*,CLEANLOCALSTORAGE*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetVarDesc_Proxy(LPTYPEINFO,UINT,LPVARDESC*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetVarDesc_Stub(LPTYPEINFO,UINT,LPVARDESC*,CLEANLOCALSTORAGE*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetNames_Proxy(LPTYPEINFO,MEMBERID,LPBSTR,UINT,LPUINT);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetNames_Stub(LPTYPEINFO,MEMBERID,LPBSTR,UINT,LPUINT);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetIDsOfNames_Proxy(LPTYPEINFO,LPOLESTR*,UINT,MEMBERID*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetIDsOfNames_Stub(LPTYPEINFO);
HRESULT STDMETHODCALLTYPE ITypeInfo_Invoke_Proxy(LPTYPEINFO,PVOID,MEMBERID,WORD,DISPPARAMS*,LPVARIANT,LPEXCEPINFO,LPUINT);
HRESULT STDMETHODCALLTYPE ITypeInfo_Invoke_Stub(LPTYPEINFO);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetDocumentation_Proxy(LPTYPEINFO,MEMBERID,LPBSTR,LPBSTR,LPDWORD,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetDocumentation_Stub(LPTYPEINFO,MEMBERID,DWORD,LPBSTR,LPBSTR,LPDWORD,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetDllEntry_Proxy(LPTYPEINFO,MEMBERID,INVOKEKIND,LPBSTR,LPBSTR,LPWORD);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetDllEntry_Stub(LPTYPEINFO,MEMBERID,INVOKEKIND,DWORD,LPBSTR,LPBSTR,LPWORD);
HRESULT STDMETHODCALLTYPE ITypeInfo_AddressOfMember_Proxy(LPTYPEINFO,MEMBERID,INVOKEKIND,PVOID*);
HRESULT STDMETHODCALLTYPE ITypeInfo_AddressOfMember_Stub(LPTYPEINFO);
HRESULT STDMETHODCALLTYPE ITypeInfo_CreateInstance_Proxy(LPTYPEINFO,LPUNKNOWN,REFIID,PVOID*);
HRESULT STDMETHODCALLTYPE ITypeInfo_CreateInstance_Stub(LPTYPEINFO,REFIID,LPUNKNOWN*);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetContainingTypeLib_Proxy(LPTYPEINFO,LPTYPELIB*,PUINT);
HRESULT STDMETHODCALLTYPE ITypeInfo_GetContainingTypeLib_Stub(LPTYPEINFO,LPTYPELIB*,PUINT);
void STDMETHODCALLTYPE ITypeInfo_ReleaseTypeAttr_Proxy(LPTYPEINFO,LPTYPEATTR);
HRESULT STDMETHODCALLTYPE ITypeInfo_ReleaseTypeAttr_Stub(LPTYPEINFO);
void STDMETHODCALLTYPE ITypeInfo_ReleaseFuncDesc_Proxy(LPTYPEINFO,LPFUNCDESC);
HRESULT STDMETHODCALLTYPE ITypeInfo_ReleaseFuncDesc_Stub(LPTYPEINFO);
void STDMETHODCALLTYPE ITypeInfo_ReleaseVarDesc_Proxy(LPTYPEINFO,VARDESC*);
HRESULT STDMETHODCALLTYPE ITypeInfo_ReleaseVarDesc_Stub(LPTYPEINFO);
HRESULT STDMETHODCALLTYPE ITypeInfo2_GetDocumentation2_Proxy(LPTYPEINFO2,MEMBERID,LCID,LPBSTR,LPDWORD,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeInfo2_GetDocumentation2_Stub(LPTYPEINFO2,MEMBERID,LCID,DWORD,LPBSTR,LPDWORD,LPBSTR);
UINT STDMETHODCALLTYPE ITypeLib_GetTypeInfoCount_Proxy(LPTYPELIB);
HRESULT STDMETHODCALLTYPE ITypeLib_GetTypeInfoCount_Stub(LPTYPELIB,PUINT);
HRESULT STDMETHODCALLTYPE ITypeLib_GetLibAttr_Proxy(LPTYPELIB,LPTLIBATTR*);
HRESULT STDMETHODCALLTYPE ITypeLib_GetLibAttr_Stub(LPTYPELIB,LPTLIBATTR*,CLEANLOCALSTORAGE*);
HRESULT STDMETHODCALLTYPE ITypeLib_GetDocumentation_Proxy(LPTYPELIB,INT,LPBSTR,LPBSTR,LPDWORD,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeLib_GetDocumentation_Stub(LPTYPELIB,INT,DWORD,LPBSTR,LPBSTR,LPDWORD,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeLib_IsName_Proxy(LPTYPELIB,LPOLESTR,ULONG,LPBOOL);
HRESULT STDMETHODCALLTYPE ITypeLib_IsName_Stub(LPTYPELIB,LPOLESTR,ULONG,LPBOOL,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeLib_FindName_Proxy(LPTYPELIB,LPOLESTR,ULONG,LPTYPEINFO*,MEMBERID*,PUSHORT);
HRESULT STDMETHODCALLTYPE ITypeLib_FindName_Stub(LPTYPELIB,LPOLESTR,ULONG,LPTYPEINFO*,MEMBERID*,PUSHORT,LPBSTR);
void STDMETHODCALLTYPE ITypeLib_ReleaseTLibAttr_Proxy(LPTYPELIB,LPTLIBATTR);
HRESULT STDMETHODCALLTYPE ITypeLib_ReleaseTLibAttr_Stub(LPTYPELIB);
HRESULT STDMETHODCALLTYPE ITypeLib2_GetLibStatistics_Proxy(LPTYPELIB2,PULONG,PULONG);
HRESULT STDMETHODCALLTYPE ITypeLib2_GetLibStatistics_Stub(LPTYPELIB2,PULONG,PULONG);
HRESULT STDMETHODCALLTYPE ITypeLib2_GetDocumentation2_Proxy(LPTYPELIB2,INT,LCID,LPBSTR,LPDWORD,LPBSTR);
HRESULT STDMETHODCALLTYPE ITypeLib2_GetDocumentation2_Stub(LPTYPELIB2,INT,LCID,DWORD,LPBSTR,LPDWORD,LPBSTR);

#ifdef __cplusplus
}
#endif
#endif
