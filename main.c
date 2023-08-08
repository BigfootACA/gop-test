#include<Uefi.h>
#include<Library/UefiLib.h>
#include<Library/BaseLib.h>
#include<Library/PrintLib.h>
#include<Library/BaseMemoryLib.h>
#include<Library/UefiBootServicesTableLib.h>
#include<Protocol/GraphicsOutput.h>
#include"image.h"

STATIC EFI_STATUS DrawImage(
	IN EFI_GRAPHICS_OUTPUT_PROTOCOL*Gop,
	IN image_dsc*Image
){
	UINT32 Left,Top;
	UINT32 Width,Height;
	if(!Image||Image->size<=0||!Image->image){
		Print(L"Invalid image to draw\n");
		return EFI_INVALID_PARAMETER;
	}
	Print(L"Image size: %ux%u\n",Image->w,Image->h);
	if(
		Image->w<=0||Image->h<=0||Image->bpp<=0||
		Image->w*Image->h*Image->bpp>Image->size
	){
		Print(L"Bad image size\n");
		return EFI_INVALID_PARAMETER;
	}
	if(Image->bpp!=4){
		Print(L"Unsupport image bpp\n");
		return EFI_INVALID_PARAMETER;
	}
	if(!Gop||!Gop->Mode||!Gop->Mode->Info){
		Print(L"Invalid Graphics Output Protocol\n");
		return EFI_INVALID_PARAMETER;
	}
	Width=Gop->Mode->Info->HorizontalResolution;
	Height=Gop->Mode->Info->VerticalResolution;
	Print(L"Screen size: %ux%u\n",Width,Height);
	Print(L"Screen pixel format: %u\n",Gop->Mode->Info->PixelFormat);
	Print(L"Screen line: %u\n",Gop->Mode->Info->PixelsPerScanLine);
	if(Width<=Image->w||Height<=Image->h){
		Print(L"Screen too small to display image\n");
		return EFI_OUT_OF_RESOURCES;
	}
	Left=(Width-Image->w)/2;
	Top=(Height-Image->h)/2;
	Print(L"Draw position: %ux%u\n",Left,Top);
	return Gop->Blt(
		Gop,
		(void*)Image->image,
		EfiBltBufferToVideo,
		0,0,Left,Top,
		Image->w,Image->h,
		Image->w*Image->bpp
	);
}

STATIC VOID WaitForAnyKey(){
	UINTN Index;
	EFI_STATUS Status;
	EFI_INPUT_KEY Key;
	if(gST->ConIn){
		do{Status=gST->ConIn->ReadKeyStroke(gST->ConIn,&Key);}
		while(!EFI_ERROR(Status));
		Print(L"Press any key to continue\n");
		gBS->WaitForEvent(1,&gST->ConIn->WaitForKey,&Index);
		do{Status=gST->ConIn->ReadKeyStroke(gST->ConIn,&Key);}
		while(!EFI_ERROR(Status));
	}else gBS->Stall(5*1000*1000);
}

STATIC VOID ClearScreen(IN EFI_GRAPHICS_OUTPUT_PROTOCOL*Gop){
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color;
	ZeroMem(&Color,sizeof(Color));
	Gop->Blt(
		Gop,&Color,
		EfiBltVideoFill,
		0,0,0,0,
		Gop->Mode->Info->HorizontalResolution,
		Gop->Mode->Info->VerticalResolution,
		0
	);
}

EFI_STATUS EFIAPI UefiMain(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE*SystemTable
){
	UINTN Size;
	EFI_STATUS Status;
	UINT32 Modes=0,Mode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL*Gop;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION*Info;
	Status=gBS->LocateProtocol(
		&gEfiGraphicsOutputProtocolGuid,
		NULL,(VOID**)&Gop
	);
	if(EFI_ERROR(Status)){
		Print(L"Locate Graphics Output Protocol Failed: %r\n",Status);
		return Status;
	}
	ClearScreen(Gop);
	Mode=Gop->Mode->Mode;
	Print(L"Test in Default Mode (%u)\n",Mode);
	Status=DrawImage(Gop,&default_image);
	if(EFI_ERROR(Status))Print(L"Draw Image Failed: %r\n",Status);
	else Modes++;
	for(UINT32 i=0;i<Gop->Mode->MaxMode;i++){
		Size=sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),Info=NULL;
		Status=Gop->QueryMode(Gop,i,&Size,&Info);
		if(EFI_ERROR(Status)||!Info)continue;
		Status=Gop->SetMode(Gop,i);
		if(EFI_ERROR(Status)){
			Print(L"Set Mode %u Failed: %r\n",i,Status);
			goto Next;
		}
		ClearScreen(Gop);
		Print(
			L"Test in Mode %u: %ux%u Format %u \n",i,
			Info->HorizontalResolution,
			Info->VerticalResolution,
			Info->PixelFormat
		);
		Print(
			L"Pixel Mask: R%08X G%08X B%08X A%08X\n",
			Info->PixelInformation.RedMask,
			Info->PixelInformation.GreenMask,
			Info->PixelInformation.BlueMask,
			Info->PixelInformation.ReservedMask
		);
		Status=DrawImage(Gop,&default_image);
		if(EFI_ERROR(Status)){
			Print(L"Draw Image Failed: %r\n",Status);
			goto Next;
		}
		Modes++;
		Next:
		gBS->Stall(1*1000*1000);
		WaitForAnyKey();
	}
	Gop->SetMode(Gop,Mode);
	ClearScreen(Gop);
	Print(L"Tested %u modes\n",Modes);
	WaitForAnyKey();
	Print(L"Exiting Graphics Output Protocol Test App...\n");
	return EFI_SUCCESS;
}
