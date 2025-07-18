#ifndef CC_PLATFORM_H
#define CC_PLATFORM_H
#include "Core.h"
CC_BEGIN_HEADER

/* 
Abstracts platform specific memory management, I/O, etc
Copyright 2014-2025 ClassiCube | Licensed under BSD-3
*/

#if defined CC_BUILD_WIN || defined CC_BUILD_XBOX
#define _NL "\r\n"
#define NATIVE_STR_LEN 300
#else
#define _NL "\n"
#define NATIVE_STR_LEN 600
#endif

/* Suffix added to app name sent to the server */
extern const char* Platform_AppNameSuffix;
void* TempMem_Alloc(int size);

#if defined CC_BUILD_WIN
typedef struct cc_winstring_ {
	cc_unichar uni[NATIVE_STR_LEN]; /* String represented using UTF16 format */
	char ansi[NATIVE_STR_LEN]; /* String lossily represented using ANSI format */
} cc_winstring;

/* Encodes a string into the platform native string format */
void Platform_EncodeString(cc_winstring* dst, const cc_string* src);
#endif

#ifdef CC_BUILD_WIN
typedef cc_winstring cc_filepath;
#else
typedef struct cc_filepath_ { char buffer[NATIVE_STR_LEN]; } cc_filepath;
#define FILEPATH_RAW(raw) ((cc_filepath*)raw)
#endif
/* Converts the provided path into a platform native file path */
void Platform_EncodePath(cc_filepath* dst, const cc_string* src);

/* Initialises the platform specific state. */
void Platform_Init(void);
/* Frees the platform specific state. */
void Platform_Free(void);
/* Sets the appropriate default current/working directory. */
cc_result Platform_SetDefaultCurrentDirectory(int argc, char **argv);
/* Gets the command line arguments passed to the program. */
int Platform_GetCommandLineArgs(int argc, STRING_REF char** argv, cc_string* args);


/*########################################################################################################################*
*--------------------------------------------------Cryptographic functions------------------------------------------------*
*#########################################################################################################################*/
/* Encrypts data in a platform-specific manner. (may not be supported) */
cc_result Platform_Encrypt(const void* data, int len, cc_string* dst);
/* Decrypts data in a platform-specific manner. (may not be supported) */
cc_result Platform_Decrypt(const void* data, int len, cc_string* dst);
/* Securely generates len bytes of random data */
cc_result Platform_GetEntropy(void* data, int len);


/*########################################################################################################################*
*--------------------------------------------------Process/Shell functions------------------------------------------------*
*#########################################################################################################################*/
/* Whether the launcher and game must both be run in the same process */
/*  (e.g. can't start a separate process on Mobile or Consoles) */
#define PLAT_FLAG_SINGLE_PROCESS 0x01
/* Whether button in the launcher should be available to exit the app */
/*  (e.g. necessary in MS DOS, game consoles ) */
#define PLAT_FLAG_APP_EXIT       0x02

/* Platform specific runtime behaviour flags (See PLAT_FLAG members) */
extern cc_uint8 Platform_Flags;
#define Platform_IsSingleProcess() (Platform_Flags & PLAT_FLAG_SINGLE_PROCESS)

/* Starts the game with the given arguments. */
CC_API cc_result Process_StartGame2(const cc_string* args, int numArgs);
/* Terminates the process with the given return code. */
CC_API void Process_Exit(cc_result code);
/* Starts the platform-specific program to open the given url or filename. */
/* For example, provide a http:// url to open a website in the user's web browser. */
CC_API cc_result Process_StartOpen(const cc_string* args);
/* Whether opening URLs is supported by the platform */
extern cc_bool Process_OpenSupported;


/*########################################################################################################################*
*-----------------------------------------------------Updater functions---------------------------------------------------*
*#########################################################################################################################*/
#define UPDATE_FILE "ClassiCube.update"

struct UpdaterBuild { 
	const char* name; 
	const char* path; 
};
extern const struct UpdaterInfo {
	const char* info;
	/* Number of compiled builds available for this platform */
	int numBuilds;
	/* Metadata for the compiled builds available for this platform */
	const struct UpdaterBuild builds[2]; /* TODO name and path */
} Updater_Info;
/* Whether updating is supported by the platform */
extern cc_bool Updater_Supported;

/* Attempts to clean up any leftover files from an update */
cc_bool Updater_Clean(void);
/* Starts the platform-specific method to update then start the game using the UPDATE_FILE file. */
/* If an error occurs, action indicates which part of the updating process failed. */
cc_result Updater_Start(const char** action);
/* Returns the last time the application was modified, as a unix timestamp. */
cc_result Updater_GetBuildTime(cc_uint64* timestamp);
/* Marks the UPDATE_FILE file as being executable. (Needed for some platforms) */
cc_result Updater_MarkExecutable(void);
/* Sets the last time UPDATE_FILE file was modified, as a unix timestamp. */
cc_result Updater_SetNewBuildTime(cc_uint64 timestamp);


/*########################################################################################################################*
*-----------------------------------------------------Dynamic libraries---------------------------------------------------*
*#########################################################################################################################*/
/* TODO: Rename _Load2 to _Load on next plugin API version */
/* Attempts to load a native dynamic library from the given path. */
CC_API void* DynamicLib_Load2(const cc_string* path);
/* Attempts to get the address of the symbol in the given dynamic library. */
/* NOTE: Do NOT use this to load OpenGL functions, use GLContext_GetAddress. */
CC_API void* DynamicLib_Get2(void* lib, const char* name);
/* Outputs more detailed information about errors with the DynamicLib functions. */
/* NOTE: You MUST call this immediately after DynamicLib_Load2/DynamicLib_Get2 returns NULL. */
CC_API cc_bool DynamicLib_DescribeError(cc_string* dst);

/* The default file extension used for dynamic libraries on this platform. */
extern const cc_string DynamicLib_Ext;
CC_API cc_result DynamicLib_Load(const cc_string* path, void** lib); /* OBSOLETE */
CC_API cc_result DynamicLib_Get(void* lib, const char* name, void** symbol); /* OBSOLETE */

#define DYNAMICLIB_QUOTE(x) #x
#define DynamicLib_ReqSym(sym) { DYNAMICLIB_QUOTE(sym), (void**)&_ ## sym, true  }
#define DynamicLib_OptSym(sym) { DYNAMICLIB_QUOTE(sym), (void**)&_ ## sym, false }

#define DynamicLib_ReqSym2(name, sym) { name,           (void**)&_ ## sym, true  }
#define DynamicLib_OptSym2(name, sym) { name,           (void**)&_ ## sym, false }

#if defined CC_BUILD_OS2
#define DynamicLib_ReqSymC(sym) { DYNAMICLIB_QUOTE(_ ## sym), (void**)&_ ## sym, true  }
#define DynamicLib_OptSymC(sym) { DYNAMICLIB_QUOTE(_ ## sym), (void**)&_ ## sym, false }
#endif

/* Contains a name and a pointer to variable that will hold the loaded symbol */
/*  static int (APIENTRY *_myGetError)(void); --- (for example) */
/*  static struct DynamicLibSym sym = { "myGetError", (void**)&_myGetError, true }; */
struct DynamicLibSym { const char* name; void** symAddr; cc_bool required; };
/* Loads all symbols using DynamicLib_Get2 in the given list */
/* Returns true if all required symbols were successfully retrieved */
cc_bool DynamicLib_LoadAll(const cc_string* path, const struct DynamicLibSym* syms, int count, void** lib);


/*########################################################################################################################*
*-------------------------------------------------------Crash handling----------------------------------------------------*
*#########################################################################################################################*/
/* Attempts to install a callback for the operating system's unhandled error event/signal. */
/* This is used to attempt to log some information about a crash due to invalid memory read, etc. */
void CrashHandler_Install(void);
/* Displays a message box with raw_msg body, logs state to disc, then immediately terminates/quits. */
/* Typically called when an unrecoverable error occurs. (e.g. out of memory) */
#define Process_Abort(msg) Process_Abort2(0, msg);
/* Displays a message box with raw_msg body, logs state to disc, then immediately terminates/quits. */
/* Typically called when an unrecoverable error occurs. (e.g. out of memory) */
CC_NOINLINE void Process_Abort2(cc_result result, const char* raw_msg);


/*########################################################################################################################*
*-----------------------------------------------------Memory allocation---------------------------------------------------*
*#########################################################################################################################*/
/* Allocates a block of memory, with undetermined contents. Returns NULL on allocation failure. */
CC_API void* Mem_TryAlloc(cc_uint32 numElems, cc_uint32 elemsSize);
/* Allocates a block of memory, with contents of all 0. Returns NULL on allocation failure. */
CC_API void* Mem_TryAllocCleared(cc_uint32 numElems, cc_uint32 elemsSize);
/* Reallocates a block of memory, with undetermined contents. Returns NULL on reallocation failure. */
CC_API void* Mem_TryRealloc(void* mem, cc_uint32 numElems, cc_uint32 elemsSize);

/* Allocates a block of memory, with undetermined contents. Exits process on allocation failure. */
CC_API void* Mem_Alloc(cc_uint32 numElems, cc_uint32 elemsSize, const char* place);
/* Allocates a block of memory, with contents of all 0. Exits process on allocation failure. */
CC_API void* Mem_AllocCleared(cc_uint32 numElems, cc_uint32 elemsSize, const char* place);
/* Reallocates a block of memory, with undetermined contents. Exits process on reallocation failure. */
CC_API void* Mem_Realloc(void* mem, cc_uint32 numElems, cc_uint32 elemsSize, const char* place);
/* Frees an allocated a block of memory. Does nothing when passed NULL. */
CC_API void  Mem_Free(void* mem);


/*########################################################################################################################*
*----------------------------------------------------Memory modification--------------------------------------------------*
*#########################################################################################################################*/
/* Sets the contents of a block of memory to the given value. */
void* Mem_Set(void* dst, cc_uint8 value, unsigned numBytes);
/* Copies a block of memory to another block of memory. */
/* NOTE: These blocks MUST NOT overlap. */
void* Mem_Copy(void* dst, const void* src, unsigned numBytes);
/* Moves a block of memory to another block of memory. */
/* NOTE: These blocks can overlap. */
void* Mem_Move(void* dst, const void* src, unsigned numBytes);
/* Returns non-zero if the two given blocks of memory have equal contents. */
int Mem_Equal(const void* a, const void* b, cc_uint32 numBytes);


/*########################################################################################################################*
*-----------------------------------------------------Logging functions---------------------------------------------------*
*#########################################################################################################################*/
/* Logs a debug message to console. */
void Platform_Log(const char* msg, int len);
void Platform_LogConst(const char* message);
void Platform_Log1(const char* format, const void* a1);
void Platform_Log2(const char* format, const void* a1, const void* a2);
void Platform_Log3(const char* format, const void* a1, const void* a2, const void* a3);
void Platform_Log4(const char* format, const void* a1, const void* a2, const void* a3, const void* a4);

/* Outputs more detailed information about errors with operating system functions. */
/* NOTE: This is for general functions like file I/O. If a more specific 
describe exists (e.g. Http_DescribeError), that should be preferred. */
cc_bool Platform_DescribeError(cc_result res, cc_string* dst);


/*########################################################################################################################*
*------------------------------------------------------Time functions-----------------------------------------------------*
*#########################################################################################################################*/
/* Number of seconds since 01/01/0001 to start of unix time. */
#define UNIX_EPOCH_SECONDS 62135596800ULL
struct cc_datetime;

/* Returns the current UTC time, as number of seconds since 1/1/0001 */
CC_API TimeMS DateTime_CurrentUTC(void);
/* Returns the current local Time. */
CC_API void DateTime_CurrentLocal(struct cc_datetime* t);
/* Takes a platform-specific stopwatch measurement. */
/* NOTE: The value returned is platform-specific - do NOT try to interpret the value. */
CC_API cc_uint64 Stopwatch_Measure(void);
/* Returns total elapsed microseconds between two stopwatch measurements. */
CC_API cc_uint64 Stopwatch_ElapsedMicroseconds(cc_uint64 beg, cc_uint64 end);
/* Returns total elapsed milliseconds between two stopwatch measurements. */
int Stopwatch_ElapsedMS(cc_uint64 beg, cc_uint64 end);


/*########################################################################################################################*
*---------------------------------------------------------File I/O--------------------------------------------------------*
*#########################################################################################################################*/
#if defined CC_BUILD_WIN || defined CC_BUILD_XBOX
typedef void* cc_file;
#else
typedef int cc_file;
#endif

/* Origin points for when seeking in a file. */
/*  NOTE: These have same values as SEEK_SET/SEEK_CUR/SEEK_END, do not change them */
enum File_SeekFrom { FILE_SEEKFROM_BEGIN, FILE_SEEKFROM_CURRENT, FILE_SEEKFROM_END };
/* Whether the filesystem is readonly (i.e. cannot make chat logs, cache, etc) */
extern cc_bool Platform_ReadonlyFilesystem;

/* Result code for when trying to open a file locked by another process */
extern const cc_result ReturnCode_FileShareViolation;
/* Result code for when trying to open a non-existent file */
extern const cc_result ReturnCode_FileNotFound;
/* Result code for when trying to create an already existent directory */
extern const cc_result ReturnCode_DirectoryExists;

/* Attempts to create a new directory. */
cc_result Directory_Create(const cc_filepath* path);
/* Callback function invoked for each file found. */
typedef void (*Directory_EnumCallback)(const cc_string* filename, void* obj, int isDirectory);
/* Invokes a callback function on all filenames in the given directory (and its sub-directories) */
CC_API cc_result Directory_Enum(const cc_string* path, void* obj, Directory_EnumCallback callback);
/* Returns non-zero if the given file exists. */
int File_Exists(const cc_filepath* path);
void Directory_GetCachePath(cc_string* path);

/* Attempts to create a new (or overwrite) file for writing. */
/* NOTE: If the file already exists, its contents are discarded. */
cc_result File_Create(cc_file* file, const cc_filepath* path);
/* Attempts to open an existing file for reading. */
cc_result File_Open(cc_file* file, const cc_filepath* path);
/* Attempts to open an existing or create a new file for reading and writing. */
cc_result File_OpenOrCreate(cc_file* file, const cc_filepath* path);
/* Attempts to read data from the file. */
cc_result File_Read(cc_file file, void* data, cc_uint32 count, cc_uint32* bytesRead);
/* Attempts to write data to the file. */
cc_result File_Write(cc_file file, const void* data, cc_uint32 count, cc_uint32* bytesWrote);
/* Attempts to close the given file. */
cc_result File_Close(cc_file file);
/* Attempts to seek to a position in the given file. */
cc_result File_Seek(cc_file file, int offset, int seekType);
/* Attempts to get the current position in the given file. */
cc_result File_Position(cc_file file, cc_uint32* pos);
/* Attempts to retrieve the length of the given file. */
cc_result File_Length(cc_file file, cc_uint32* len);


/*########################################################################################################################*
*---------------------------------------------------------Threading-------------------------------------------------------*
*#########################################################################################################################*/
typedef void (*Thread_StartFunc)(void);
/* Blocks the current thread for the given number of milliseconds. */
CC_API void Thread_Sleep(cc_uint32 milliseconds);
/* Initialises and starts a new thread that runs the given function. */
/* NOTE: Threads must either be detached or joined, otherwise data leaks. */
CC_API void Thread_Run(void** handle, Thread_StartFunc func, int stackSize, const char* name);
/* Frees the platform specific persistent data associated with the thread. */
/* NOTE: Once a thread has been detached, Thread_Join can no longer be used. */
CC_API void Thread_Detach(void* handle);
/* Blocks the current thread, until the given thread has finished. */
/* NOTE: This cannot be used on a thread that has been detached. */
CC_API void Thread_Join(void* handle);


/*########################################################################################################################*
*-----------------------------------------------------------Mutex---------------------------------------------------------*
*#########################################################################################################################*/
/* Allocates a new mutex. (used to synchronise access to a shared resource) */
CC_API void* Mutex_Create(const char* name);
/* Frees an allocated mutex. */
CC_API void  Mutex_Free(void* handle);
/* Locks the given mutex, blocking other threads from entering. */
CC_API void  Mutex_Lock(void* handle);
/* Unlocks the given mutex, allowing other threads to enter. */
CC_API void  Mutex_Unlock(void* handle);


/*########################################################################################################################*
*---------------------------------------------------------Waitable--------------------------------------------------------*
*#########################################################################################################################*/
/* Allocates a new waitable. (used to conditionally wake-up a blocked thread) */
CC_API void* Waitable_Create(const char* name);
/* Frees an allocated waitable. */
CC_API void  Waitable_Free(void* handle);
/* Signals a waitable, waking up blocked threads. */
CC_API void  Waitable_Signal(void* handle);
/* Blocks the calling thread until the waitable gets signalled. */
CC_API void  Waitable_Wait(void* handle);
/* Blocks the calling thread until the waitable gets signalled, or milliseconds delay passes. */
CC_API void  Waitable_WaitFor(void* handle, cc_uint32 milliseconds);

/* Calls SysFonts_Register on each font that is available on this platform. */
void Platform_LoadSysFonts(void);


/*########################################################################################################################*
*----------------------------------------------------------Sockets--------------------------------------------------------*
*#########################################################################################################################*/
#if defined CC_BUILD_WIN || defined CC_BUILD_XBOX
typedef cc_uintptr cc_socket;
#else
typedef int cc_socket;
#endif
#define CC_SOCKETADDR_MAXSIZE 512
#define SOCKET_MAX_ADDRS 5

typedef struct cc_sockaddr_ {
	int size; /* Actual size of the raw socket address */
	cc_uint8 data[CC_SOCKETADDR_MAXSIZE]; /* Raw socket address (e.g. sockaddr_in) */
} cc_sockaddr;

enum Socket_PollMode { SOCKET_POLL_READ, SOCKET_POLL_WRITE };

/* Result code for when a socket operation is aynchronously executing */
extern const cc_result ReturnCode_SocketInProgess;
/* Result code for when a socket operation would block the calling thread */
extern const cc_result ReturnCode_SocketWouldBlock;
/* Result code for when a socket connection has been dropped by the other side */
extern const cc_result ReturnCode_SocketDropped;

/* Checks if the given socket is currently readable (i.e. has data available to read) */
/* NOTE: A closed socket is also considered readable */
cc_result Socket_CheckReadable(cc_socket s, cc_bool* readable);
/* Checks if the given socket is currently writable (i.e. has finished connecting) */
cc_result Socket_CheckWritable(cc_socket s, cc_bool* writable);
/* If the input represents an IP address, then parses the input into a single IP address */
/* Otherwise, attempts to resolve the input via DNS into one or more IP addresses */
cc_result Socket_ParseAddress(const cc_string* address, int port, cc_sockaddr* addrs, int* numValidAddrs);

/* Allocates a new socket that is capable of connecting to the given address */
cc_result Socket_Create(cc_socket* s, cc_sockaddr* addr, cc_bool nonblocking);
/* Begins connecting to the given address */
cc_result Socket_Connect(cc_socket s, cc_sockaddr* addr);
/* Attempts to read data from the given socket */
/* NOTE: A closed socket may set modified to 0, but still return 'success' (i.e. 0) */
cc_result Socket_Read(cc_socket s, cc_uint8* data, cc_uint32 count, cc_uint32* modified);
/* Attempts to write data to the given socket */
cc_result Socket_Write(cc_socket s, const cc_uint8* data, cc_uint32 count, cc_uint32* modified);
/* Attempts to close the given socket */
void Socket_Close(cc_socket s);
/* Attempts to write all data to the given socket, returning ERR_END_OF_STREAM if it could not */
cc_result Socket_WriteAll(cc_socket socket, const cc_uint8* data, cc_uint32 count);


/*########################################################################################################################*
*-----------------------------------------------------Platform specific---------------------------------------------------*
*#########################################################################################################################*/
#ifdef CC_BUILD_MOBILE
void Platform_ShareScreenshot(const cc_string* filename);
#endif

#ifdef CC_BUILD_ANDROID
#include <jni.h> /* TODO move to interop file */
extern jclass  App_Class;
extern jobject App_Instance;
extern JavaVM* VM_Ptr;
void Platform_TryLogJavaError(void);

#define JavaGetCurrentEnv(env) (*VM_Ptr)->AttachCurrentThread(VM_Ptr, &env, NULL)
#define JavaMakeConst(env, str) (*env)->NewStringUTF(env, str)

#define JavaRegisterNatives(env, methods) (*env)->RegisterNatives(env, App_Class, methods, Array_Elems(methods));
#define JavaGetIMethod(env, name, sig) (*env)->GetMethodID(env, App_Class, name, sig)
#define JavaGetSMethod(env, name, sig) (*env)->GetStaticMethodID(env, App_Class, name, sig)

/* Creates a string from the given java string. buffer must be at least NATIVE_STR_LEN long. */
/* NOTE: Don't forget to call env->ReleaseStringUTFChars. Only works with ASCII strings. */
cc_string JavaGetString(JNIEnv* env, jstring str, char* buffer);
/* Allocates a java string from the given string. */
jobject JavaMakeString(JNIEnv* env, const cc_string* str);
/* Allocates a java byte array from the given block of memory. */
jbyteArray JavaMakeBytes(JNIEnv* env, const void* src, cc_uint32 len);
/* Calls a method in the activity class that returns nothing. */
void JavaCallVoid(JNIEnv*  env, const char* name, const char* sig, jvalue* args);
/* Calls a method in the activity class that returns a jint. */
jlong JavaCallLong(JNIEnv* env, const char* name, const char* sig, jvalue* args);
/* Calls a method in the activity class that returns a jobject. */
jobject JavaCallObject(JNIEnv* env, const char* name, const char* sig, jvalue* args);
/* Calls a method in the activity class that takes a string and returns nothing. */
void JavaCall_String_Void(const char* name, const cc_string* value);
/* Calls a method in the activity class that takes no arguments and returns a string. */
void JavaCall_Void_String(const char* name, cc_string* dst);
/* Calls a method in the activity class that takes a string and returns a string. */
void JavaCall_String_String(const char* name, const cc_string* arg, cc_string* dst);

/* Calls an instance method in the activity class that returns nothing */
#define JavaICall_Void(env, method, args) (*env)->CallVoidMethodA(env,  App_Instance, method, args)
/* Calls an instance method in the activity class that returns a jint */
#define JavaICall_Int(env,  method, args) (*env)->CallIntMethodA(env,   App_Instance, method, args)
/* Calls an instance method in the activity class that returns a jlong */
#define JavaICall_Long(env, method, args) (*env)->CallLongMethodA(env,  App_Instance, method, args)
/* Calls an instance method in the activity class that returns a jfloat */
#define JavaICall_Float(env,method, args) (*env)->CallFloatMethodA(env, App_Instance, method, args)
/* Calls an instance method in the activity class that returns a jobject */
#define JavaICall_Obj(env,  method, args) (*env)->CallObjectMethodA(env,App_Instance, method, args)

/* Calls a static method in the activity class that returns nothing */
#define JavaSCall_Void(env, method, args) (*env)->CallStaticVoidMethodA(env,  App_Class, method, args)
/* Calls a static method in the activity class that returns a jint */
#define JavaSCall_Int(env,  method, args) (*env)->CallStaticIntMethodA(env,   App_Class, method, args)
/* Calls a static method in the activity class that returns a jlong */
#define JavaSCall_Long(env, method, args) (*env)->CallStaticLongMethodA(env,  App_Class, method, args)
/* Calls a static method in the activity class that returns a jfloat */
#define JavaSCall_Float(env,method, args) (*env)->CallStaticFloatMethodA(env, App_Class, method, args)
/* Calls a static method in the activity class that returns a jobject */
#define JavaSCall_Obj(env,  method, args) (*env)->CallStaticObjectMethodA(env,App_Class, method, args)
#endif

CC_END_HEADER
#endif
