/*
  Cockos WDL - LICE - Lightweight Image Compositing Engine
  Copyright (C) 2007 and later, Cockos Incorporated
  File: main.cpp (example use of LICE)
  See lice.h for license and other information
*/

#include "../lice.h"
#include "../../plush2/plush.h"
#include <math.h>
#include <stdio.h>

#include "resource.h"

#define NUM_EFFECTS 18

HINSTANCE g_hInstance;
LICE_IBitmap *jpg;
LICE_IBitmap *bmp;
LICE_IBitmap *icon;
LICE_SysBitmap *framebuffer;
static int m_effect = 17;
static int m_doeff = 0;

static DWORD m_start_time, m_frame_cnt;

static void DoPaint(HWND hwndDlg)
{
  PAINTSTRUCT ps;
  
  HDC dc = BeginPaint(hwndDlg, &ps);
  RECT r;
  GetClientRect(hwndDlg, &r);
  
#ifdef _WIN32
  r.top+=40;
  if (r.top >= r.bottom) r.top=r.bottom-1;
#endif
  
  if (framebuffer->resize(r.right-r.left,r.bottom-r.top))
  {
    m_doeff=1;
    memset(framebuffer->getBits(),0,framebuffer->getWidth()*framebuffer->getHeight()*4);
  }
  
  int x=rand()%(r.right+300)-150;
  int y=rand()%(r.bottom+300)-150;
  
  switch(m_effect)
  {
    case 17:
      {
        static pl_Obj *obj=NULL,*obj2=NULL;
        if (!obj)
        {
          pl_Mat *mat = new pl_Mat;
          pl_Mat *mat2 = new pl_Mat;

          mat2->Smoothing=false;
          mat2->Ambient[0]=mat2->Ambient[1]=mat2->Ambient[2]=0.0;
          mat2->Diffuse[0]=mat2->Diffuse[1]=0.6;
          mat2->Diffuse[2]=1.0;

          mat->Ambient[0]=mat->Ambient[1]=mat->Ambient[2]=0.0;
          mat->Diffuse[0]=mat->Diffuse[1]=1.9;
          mat->Diffuse[2]=0.4;

          mat->PerspectiveCorrect=16;
          mat->SolidCombineMode=LICE_BLIT_MODE_COPY;
          mat->SolidOpacity=1.0;
          mat->Smoothing=true;
          mat->Lightable=true;
          //mat->FadeDist = 300.0;

          mat2->Texture=bmp;
          mat2->TexOpacity=0.5;
          mat2->TexCombineMode=LICE_BLIT_MODE_MUL|LICE_BLIT_FILTER_BILINEAR;
          mat2->SolidOpacity=0.4;
          mat2->BackfaceCull=false;
          mat2->BackfaceIllumination=1.0;

          mat->Texture=bmp;
          LICE_TexGen_Marble(mat->Texture = new LICE_MemBitmap(r.right,r.bottom),NULL,0.3,0.4,0.0,1.0f);

          mat->TexOpacity=0.5;
          mat->TexScaling[0]=mat->TexScaling[1]=3.0;
          mat->TexCombineMode=LICE_BLIT_MODE_MUL|LICE_BLIT_FILTER_BILINEAR;

          LICE_TexGen_Noise(mat->Texture2 = new LICE_MemBitmap(r.right,r.bottom),NULL,0.3,0.4,0.0,1.0f);
        //  mat->Texture2=icon;
          mat->Tex2MapIdx=-1;
          mat->Tex2CombineMode=LICE_BLIT_MODE_ADD|LICE_BLIT_FILTER_BILINEAR;
          mat->Tex2Opacity=0.8;
          mat->Tex2Scaling[0]=2.0;
          mat->Tex2Scaling[1]=-2.0;

          mat->BackfaceCull=true;
          mat->BackfaceIllumination=0.0;

          obj=plMakeTorus(100.0,80.0,40,40,mat);          

          int x;
          if (0)for(x=1;x<3;x++)
          {
            pl_Obj *no = obj->Clone();
            no->Translate(0,40.0,-x*35.0);
            obj->Children.Add(no);
            no->Xa += 50.35*x;
            no->Ya -= 30.13*x;
          }
          obj2=plMakeBox(130,130,130,mat2);

          /*pl_Obj *o = plRead3DSObj("c:\\temp\\suzanne.3ds",mat);
          if (o)
          {
            o->Scale(30.0);
            o->Translate(150.0,0,0);
            obj->Children.Add(o);
          }
          */
        }
        obj2->Xa+=0.3;
        obj2->Ya+=-0.1;
        obj->Ya+=0.1;
        obj->Xa+=0.1;
        obj->Za+=0.1;
        obj->GenMatrix=true;

        if (1) LICE_Clear(framebuffer,0);
        else {
          double a=GetTickCount()/1000.0;
        
          double scale=(1.1+sin(a)*0.3);
      
          LICE_RotatedBlit(framebuffer,framebuffer,0,0,r.right,r.bottom,0+sin(a*0.3)*16.0,0+sin(a*0.21)*16.0,r.right,r.bottom,cos(a*0.5)*0.13,false,254/255.0,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
        }
        static pl_Cam cam;
        LICE_SubBitmap tmpbm(framebuffer,10,10,framebuffer->getWidth()-20,framebuffer->getHeight()-20);
        //cam.CenterX = (tmpbm.getWidth()/2+80);
        //cam.CenterY = (tmpbm.getHeight()/2+80);
        cam.AspectRatio = 1.0;//cam.frameBuffer->getWidth()* 3.0/4.0 / (double)cam.frameBuffer->getHeight();
        cam.X = cam.Y = 0.0;
        cam.Z = -200.0;
        cam.WantZBuffer=true;
        cam.SetTarget(0,0,0);

        

        static pl_Light light;
        light.Set(PL_LIGHT_POINT,500.0,0,-900.0,1.3f,0.5f,0.5f,1000.0);
        static pl_Light light2;
        light2.Set(PL_LIGHT_POINT,-500.0,0,-700.0,0.0f,1.0f,0.5f,1000.0);
        cam.ClipBack=220.0;

        cam.Begin(&tmpbm);
        cam.RenderLight(&light);
        cam.RenderLight(&light2);
        cam.RenderObject(obj);
        cam.SortToCurrent();
        cam.RenderObject(obj2);
        cam.End();

        char buf[512];
        sprintf(buf,"tri: %d->%d->%d, pix=%.0f",
          cam.RenderTrisIn,
          cam.RenderTrisCulled,
          cam.RenderTrisOut,cam.RenderPixelsOut);
        LICE_DrawText(framebuffer,0,10,buf,LICE_RGBA(255,255,255,255),1.0f,0);

      }
    break;
    case 15:
    case 0:
    {
      double a=GetTickCount()/1000.0;
      
      double scale=(1.1+sin(a)*0.3);
      
      if (1)  // weirdness
      {
        LICE_RotatedBlit(framebuffer,framebuffer,0,0,r.right,r.bottom,0+sin(a*0.3)*16.0,0+sin(a*0.21)*16.0,r.right,r.bottom,cos(a*0.5)*0.13,false,254/255.0,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
      }
      else // artifact-free mode
      {
        LICE_MemBitmap framebuffer_back;
        
        LICE_Copy(&framebuffer_back,framebuffer);
        LICE_RotatedBlit(framebuffer,&framebuffer_back,0,0,r.right,r.bottom,0+sin(a*0.3)*16.0,0+sin(a*0.21)*16.0,r.right,r.bottom,cos(a*0.5)*0.13,false,1.0,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
      }
      //LICE_Clear(framebuffer,0);
      if (bmp) LICE_RotatedBlit(framebuffer,bmp,r.right*scale,r.bottom*scale,r.right*(1.0-scale*2.0),r.bottom*(1.0-scale*2.0),0,0,bmp->getWidth(),bmp->getHeight(),cos(a*0.3)*13.0,false,rand()%16==0 ? -0.5: 0.1,LICE_BLIT_MODE_ADD|LICE_BLIT_USE_ALPHA|LICE_BLIT_FILTER_BILINEAR);
      
      if (m_effect==15)
      {
        LICE_MultiplyAddRect(framebuffer,0,0,framebuffer->getWidth(),framebuffer->getHeight(),0.9,0.9,-0.3,1,
                             3,2,200,0);
      }
      
    }
      break;
    case 1:
      if (rand()%6==0)
        LICE_Blit(framebuffer,bmp,x,y,NULL,-1.4,LICE_BLIT_MODE_ADD|LICE_BLIT_USE_ALPHA);
      else
        LICE_Blit(framebuffer,bmp,x,y,NULL,0.6,LICE_BLIT_MODE_COPY|LICE_BLIT_USE_ALPHA);
      break;
    case 2:
    {
      LICE_Clear(framebuffer,0);
      double a=GetTickCount()/1000.0;
      
      double scale=(1.1+sin(a)*0.3);
      if (bmp) LICE_RotatedBlit(framebuffer,bmp,r.right*scale,r.bottom*scale,r.right*(1.0-scale*2.0),r.bottom*(1.0-scale*2.0),0,0,bmp->getWidth(),bmp->getHeight(),cos(a*0.3)*13.0,false,1.0,LICE_BLIT_MODE_ADD|LICE_BLIT_USE_ALPHA|LICE_BLIT_FILTER_BILINEAR,0.0,-bmp->getHeight()/2);
    }
      break;
    case 3:
    {
      //LICE_Clear(framebuffer,0);
      static double a;
      a+=0.04;
      int xsize=sin(a*3.0)*r.right*1.5;
      int ysize=sin(a*1.7)*r.bottom*1.5;
      
      if (bmp)
      {
        if (rand()%3==0)
          LICE_ScaledBlit(framebuffer,bmp,r.right/2-xsize/2,r.bottom/2-ysize/2,xsize,ysize,0.0,0.0,bmp->getWidth(),bmp->getHeight(),-0.7,LICE_BLIT_USE_ALPHA|LICE_BLIT_MODE_ADD|LICE_BLIT_FILTER_BILINEAR);
        else
          LICE_ScaledBlit(framebuffer,bmp,r.right/2-xsize/2,r.bottom/2-ysize/2,xsize,ysize,0.0,0.0,bmp->getWidth(),bmp->getHeight(),0.25,LICE_BLIT_USE_ALPHA|LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
      }
    }
      break;
    case 4:
    case 9:
      
    {
      static double a;
      a+=0.003;
      
      LICE_GradRect(framebuffer,0,0,framebuffer->getWidth(),framebuffer->getHeight(),
                    0.5*sin(a*14.0),0.5*cos(a*2.0+1.3),0.5*sin(a*4.0),1.0,
                    (cos(a*37.0))/framebuffer->getWidth()*0.5,(sin(a*17.0))/framebuffer->getWidth()*0.5,(cos(a*7.0))/framebuffer->getWidth()*0.5,0,
                    (sin(a*12.0))/framebuffer->getHeight()*0.5,(cos(a*4.0))/framebuffer->getHeight()*0.5,(cos(a*3.0))/framebuffer->getHeight()*0.5,0,
                    LICE_BLIT_MODE_COPY);
      
      
      if (m_effect==9)
      {
        /*            LOGFONT lf={
        140,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,
        "Times New Roman"
        };
        HFONT font=CreateFontIndirect(&lf);
        
        */
        
        
        LICE_SysBitmap bm(60,60);
        LICE_Clear(&bm,LICE_RGBA(0,0,0,0));
        SetTextColor(bm.getDC(),RGB(255,255,255));
        SetBkMode(bm.getDC(),TRANSPARENT);
        //            HGDIOBJ of=SelectObject(bm.getDC(),font);
        RECT r={0,0,bm.getWidth(),bm.getHeight()};
        DrawText(bm.getDC(),"LICE",-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        //        SelectObject(bm.getDC(),of);
        //          DeleteObject(font);
        
        LICE_Blit(&bm,&bm,0,0,NULL,1.0,LICE_BLIT_MODE_CHANCOPY|LICE_PIXEL_R|(LICE_PIXEL_A<<2));
        
        LICE_RotatedBlit(framebuffer,&bm,0,0,framebuffer->getWidth(),framebuffer->getHeight(),0,0,bm.getWidth(),bm.getHeight(),a*10.0,false,.4,LICE_BLIT_MODE_COPY|LICE_BLIT_USE_ALPHA|LICE_BLIT_FILTER_BILINEAR,cos(a*30.1)*10.0,sin(a*21.13)*10.0);
      }
      
      break;
    }
    case 5:
      if(m_doeff)
      {
        LICE_TexGen_Marble(framebuffer, NULL, 1, 1, 1, 1);
      }
      break;
    case 6:
      if(m_doeff)
      {
        LICE_TexGen_Noise(framebuffer, NULL, 0.9, 0.3, 0.6, 6.0f, NOISE_MODE_WOOD, 2);
      }
      break;
    case 7:
      if(m_doeff)
      {
        LICE_TexGen_Noise(framebuffer, NULL, 1,1,1, 8.0f, NOISE_MODE_NORMAL, 8);
      }
      break;
    case 8:
      if(m_doeff)
      {
        LICE_TexGen_CircNoise(framebuffer, NULL, 0.5f,0.5f,0.5f, 12.0f, 0.1f, 32);
      }
      break;
    case 10:
    {
      int x;
      static double a;
      double sc=sin(a)*0.024;
      a+=0.03;
      for (x = 0; x < 10000; x ++)
        LICE_PutPixel(framebuffer,rand()%framebuffer->getWidth(),rand()%framebuffer->getHeight(),LICE_RGBA(255,255,255,255),sc,LICE_BLIT_MODE_ADD);
    }
      break;
    case 11:
      //line test
    {
      int w = framebuffer->getWidth(), h = framebuffer->getHeight();      
      LICE_Line(framebuffer, rand()%(w*3/2)-w/4, rand()%(h*3/2)-h/4, rand()%(w*3/2)-w/4, rand()%(h*3/2)-h/4, LICE_RGBA(rand()%255,rand()%255,rand()%255,255));
    }
      break;
    case 12:
      //lice draw text test
    {
      static double a;
      a+=0.001;
      LICE_DrawText(framebuffer,0.5*(1+sin(a))*(framebuffer->getWidth()-30),0.5*(1+sin(a*7.0+1.3))*(framebuffer->getHeight()-16),"LICE RULEZ",LICE_RGBA(255,0,0,0),sin(a*0.7),LICE_BLIT_MODE_ADD);
    }
      break;
    case 13:
      //icon loading test
    {
      LICE_Clear(framebuffer, LICE_RGBA(255,255,255,255));
      LICE_Blit(framebuffer,icon,0,0,NULL,1.0f,LICE_BLIT_MODE_COPY|LICE_BLIT_USE_ALPHA);
    }
      break;
    case 14:
      // circles/arcs
    {
      int w = framebuffer->getWidth(), h = framebuffer->getHeight();
      const double _PI = acos(-1.0);
      static int m_init, m_x, m_y;
      if (!m_init) {
        m_init = true;
        m_x = w/2; m_y = h/2;
      }
      int r = rand()%w;
      float alpha = 1.0f; //(float) r / (float) w;
      float aLo = 2*_PI*rand()/RAND_MAX;
      float aHi = 2*_PI*rand()/RAND_MAX;
      
      //LICE_Clear(framebuffer, LICE_RGBA(0,0,0,0));
      LICE_Arc(framebuffer, m_x, m_y, r, aLo, aHi, LICE_RGBA(rand()%255,rand()%255,rand()%255,255),alpha);
      //LICE_Circle(framebuffer, m_x, m_y, r, LICE_RGBA(rand()%255,rand()%255,rand()%255,255));
    }
      break;
    case 16:
    {
      int sw=framebuffer->getWidth();
      int sh=framebuffer->getHeight();
      
      LICE_MemBitmap framebuffer_back;
      {
        static double a;
        a+=0.003;
        
        static int turd;
        if ((turd++&511) < 12)
          LICE_GradRect(framebuffer,sw/4,sh/4,sw/2,sh/2,
                        0.5*sin(a*14.0),0.5*cos(a*2.0+1.3),0.5*sin(a*4.0),0.1,
                        (cos(a*37.0))/framebuffer->getWidth()*0.5,(sin(a*17.0))/framebuffer->getWidth()*0.5,(cos(a*7.0))/framebuffer->getWidth()*0.5,0,
                        (sin(a*12.0))/framebuffer->getHeight()*0.5,(cos(a*4.0))/framebuffer->getHeight()*0.5,(cos(a*3.0))/framebuffer->getHeight()*0.5,0,
                        LICE_BLIT_MODE_ADD);
      }
      //LICE_TexGen_Marble(framebuffer, NULL, 1, 1, 1, 1);
      LICE_Copy(&framebuffer_back,framebuffer);
      
      
      const int divw=10;
      const int divh=5;
      float pts[2*divw*divh];
      static float angs[2*divw*divh];
      static float dangs[2*divw*divh];
      static int turd;
      if (!turd)
      {
        turd++;
        int a;
        for (a = 0; a  < 2*divw*divh; a ++)
        {
          dangs[a]=((rand()%1000)-500)*0.0001;
          angs[a]=((rand()%1000)-500)*0.1;
        }
      }
      int x,y;
      for (y=0;y<divh; y++)
      {
        for (x=0;x<divw; x ++)
        {
          int idx=(y*divw+x)*2;
          float ang=angs[idx]+=dangs[idx];
          float ang2=angs[idx+1]+=dangs[idx+1];
          pts[idx]=sw*(float)x/(float)(divw-1) + (cos(ang))*sw*0.01;
          pts[idx+1]=sh*(float)y/(float)(divh-1) + (sin(ang2))*sh*0.01;
        }
      }
      
      
      LICE_TransformBlit(framebuffer,&framebuffer_back,0,0,framebuffer->getWidth(),
                         framebuffer->getHeight(),pts,divw,divh,0.8,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR);
    }
      
      break;
      
  }
  
  if(jpg)
  {
    LICE_ScaledBlit(framebuffer,jpg,0,0,framebuffer->getWidth(),framebuffer->getHeight(),0,0,jpg->getWidth(),jpg->getHeight(),0.5,LICE_BLIT_MODE_COPY);
  }
  
  m_frame_cnt++;
  
  double sec=(GetTickCount()-m_start_time)*0.001;
  if (sec>0.0001)
  {
    char buf[512];
    sprintf(buf,"%dx%d @ %.2ffps",framebuffer->getWidth(),framebuffer->getHeight(),m_frame_cnt / (double)sec);
    LICE_DrawText(framebuffer,1,1,buf,LICE_RGBA(0,0,0,0),1,LICE_BLIT_MODE_COPY);
    LICE_DrawText(framebuffer,0,0,buf,LICE_RGBA(255,255,255,0),1,LICE_BLIT_MODE_COPY);
  }
  
  m_doeff = 0;
  
#ifndef _WIN32
  SWELL_SyncCtxFrameBuffer(framebuffer->getDC()); // flush required on OS X
#endif
  BitBlt(dc,r.left,r.top,framebuffer->getWidth(),framebuffer->getHeight(),framebuffer->getDC(),0,0,SRCCOPY);
  //      bmp->blitToDC(dc, NULL, 0, 0);
  
  EndPaint(hwndDlg, &ps);
}

// this is only used on OS X since it's way faster there
LRESULT WINAPI testRenderDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg==WM_PAINT)
  {
    DoPaint(hwndDlg);
    return 0;
  }
    
   return DefWindowProc(hwndDlg,uMsg,wParam,lParam);
}

BOOL WINAPI dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_INITDIALOG:
    
    framebuffer = new LICE_SysBitmap(0,0);
    
    jpg=LICE_LoadJPG("C:/turds.jpg");

#ifdef _WIN32
    bmp = LICE_LoadPNGFromResource(g_hInstance, IDC_PNG1);
    icon = LICE_LoadIconFromResource(g_hInstance, IDI_MAIN, 0);
#endif
      
    
    SetTimer(hwndDlg,1,3,NULL);
    {
      int x;
      for (x = 0; x < NUM_EFFECTS; x ++)
      {
        char buf[512];
        wsprintf(buf,"Effect %d",x+1);
        SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)buf);
      }
      SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,m_effect,0);

      m_start_time=GetTickCount();
      m_frame_cnt=0;
    }
  return 0;
  case WM_DESTROY:
      
      
    delete icon;
    delete bmp;
    delete framebuffer;    
  return 0;
    
#ifdef _WIN32
  case WM_TIMER:
    InvalidateRect(hwndDlg,NULL,FALSE);
    return 0;
  case WM_PAINT:
    DoPaint(hwndDlg);
    break;
#else
  case WM_SIZE:
  {
    RECT r;
    GetClientRect(hwndDlg,&r);
    r.top+=40;
    SetWindowPos(GetDlgItem(hwndDlg,IDC_RECT),NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE);
  }
  return 0;
  case WM_TIMER:
    InvalidateRect(GetDlgItem(hwndDlg,IDC_RECT),NULL,FALSE);
  return 0;
#endif
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
      case IDC_COMBO1:
        m_effect = SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
        m_doeff=1;
        m_start_time=GetTickCount();
        m_frame_cnt=0;
      break;
      case IDCANCEL:
#ifdef _WIN32
        EndDialog(hwndDlg, 0);
#else
        DestroyWindow(hwndDlg); // on mac we run modeless
#endif
      break;
    }
    break;
  }
  return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nShowCmd)
{

  g_hInstance=hInstance;
  DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, dlgProc);


  return 0;
}
#endif