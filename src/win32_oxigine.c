#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;
typedef float f32;
typedef double f64;

static b32 globalRunning;
static FILE *logFile;

LRESULT CALLBACK win32WindowsCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (uMsg) {
  case WM_CREATE: {
  } break;
  case WM_SIZE: {
  } break;
  case WM_DESTROY: {
    globalRunning = false;
  } break;
  case WM_CLOSE: {
    globalRunning = false;
  } break;
  case WM_ACTIVATEAPP: {
  } break;
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP: {
  } break;
  case WM_PAINT: {
    PAINTSTRUCT paint;
    HDC hdc = BeginPaint(hwnd, &paint);
    EndPaint(hwnd, &paint);
  } break;
  default: {
    result = DefWindowProc(hwnd, uMsg, wParam, lParam);
  } break;
  }
  return result;
}

static void win32ProcessMessages() {
  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
    switch (message.message) {
    case WM_QUIT: {
      globalRunning = false;
    } break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    default: {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    }
  }
}

/**
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderApplicationInterface.md
 */
static int win32LoadVulkan() {
  /* Indirectly Linking to the Loader */
  HMODULE vulkan = LoadLibraryW(L"vulkan-1.dll");
  if (!vulkan) {
    fprintf(logFile, "No vulkan!\n");
    return -1;
  }

  typedef PFN_vkVoidFunction vkGetInstanceProcAddrT(VkInstance, const char *);
  vkGetInstanceProcAddrT *vkGetInstanceProcAddr =
      (vkGetInstanceProcAddrT *)(GetProcAddress(vulkan, "vkGetInstanceProcAddr"));

  if (!vkGetInstanceProcAddr) {
    fprintf(logFile, "No vkGetInstanceProcAddr!\n");
    return -1;
  }

  typedef VkResult vkEnumerateInstanceLayerPropertiesT(uint32_t *, VkLayerProperties *);
  vkEnumerateInstanceLayerPropertiesT *vkEnumerateInstanceLayerProperties =
      (vkEnumerateInstanceLayerPropertiesT *)(vkGetInstanceProcAddr(
          0, "vkEnumerateInstanceLayerProperties"));
  if (!vkEnumerateInstanceLayerProperties) {
    fprintf(logFile, "No vkEnumerateInstanceLayerProperties!\n");
    return -1;
  }

  uint32_t propertyCount;
  assert(vkEnumerateInstanceLayerProperties(&propertyCount, 0) == VK_SUCCESS);
  VkLayerProperties *pProperties =
      (VkLayerProperties *)malloc(propertyCount * sizeof(VkLayerProperties));
  assert(vkEnumerateInstanceLayerProperties(&propertyCount, pProperties) == VK_SUCCESS);
  for (int i = 0; i < propertyCount; ++i) {
    fprintf(logFile, "layer %d: %s\n", i, pProperties[i].layerName);
  }
  free(pProperties);
  return 0;
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  WNDCLASS WindowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                          .lpfnWndProc = win32WindowsCallback,
                          .hInstance = hInstance,
                          //.hIcon =
                          .lpszClassName = L"OxigineWindowClass"};
  if (!RegisterClass(&WindowClass))
    exit(-1);
  HWND window = CreateWindowEx(0 /*WS_EX_TOPMOST | WS_EX_LAYERED*/, WindowClass.lpszClassName,
                               L"Oxigine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
  if (!window)
    exit(-1);

  logFile = fopen("last_run.log", "w");

  if (win32LoadVulkan() != 0) {
    exit(-1);
  }

  globalRunning = true;
  while (globalRunning) {
    win32ProcessMessages();
  }

  fclose(logFile);
}