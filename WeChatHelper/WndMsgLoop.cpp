#include "stdafx.h"
#include "WndMsgLoop.h"
#include "InitWeChat.h"
#include "Login.h"
#include "MainWindow.h"
#include "FriendList.h"
#include "ChatRecord.h"
#include "Function.h"
#include "ChatRoomOperate.h"
#include "CAutoFunction.h"
#include "SelfInformation.h"
#include <stdio.h>

extern BOOL g_AutoChat;					//�Զ�����



//************************************************************
// ��������: RegisterWindow
// ����˵��: ��ʼ������ 
// ��    ��: GuiShou
// ʱ    ��: 2019/6/30
// ��    ��: HMODULE hModule ���
// �� �� ֵ: void 
//************************************************************
void InitWindow(HMODULE hModule)
{
	//��鵱ǰ΢�Ű汾
	if (IsWxVersionValid())
	{
		//��ȡWeChatWin�Ļ�ַ
		DWORD dwWeChatWinAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll");
	
		//���΢���Ƿ��½
		DWORD dwIsLogin = dwWeChatWinAddr + LoginSign_Offset;
		if (*(DWORD*)dwIsLogin == 0)	//����0˵��΢��δ��¼
		{
			//���̳߳������΢�ŵ�½״̬
			HANDLE hThread= CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CheckIsLogin, 0, 0, NULL);
			CloseHandle(hThread);

			//HOOK��ȡ�����б���call
			HookGetFriendList();
			
			//HOOK������Ϣ
			HookChatRecord();
			
			//������
			AntiRevoke();
			
			//HOOK��ȡ���� 
			//HookExtractExpression();

			//ע�ᴰ��
			RegisterWindow(hModule);
		}
		else
		{
			//���΢���Ѿ���½ ������Ϣ���ͻ���
			HWND hLogin = FindWindow(NULL, L"Login");
			if (hLogin == NULL)
			{
				OutputDebugStringA("δ���ҵ�Login����");
				return;
			}
			COPYDATASTRUCT login_msg;
			login_msg.dwData = WM_AlreadyLogin;
			login_msg.lpData = NULL;
			login_msg.cbData = 0;
			//������Ϣ�����ƶ�
			SendMessage(hLogin, WM_COPYDATA, (WPARAM)hLogin, (LPARAM)&login_msg);
		}
	}
	else
	{
		MessageBoxA(NULL, "��ǰ΢�Ű汾��ƥ�䣬������WeChat 3.2.1.154", "����", MB_OK);
	}

}


//************************************************************
// ��������: RegisterWindow
// ����˵��: ע�ᴰ��
// ��    ��: GuiShou
// ʱ    ��: 2019/6/30
// ��    ��: HMODULE hModule ���ھ��
// �� �� ֵ: void 
//************************************************************

void RegisterWindow(HMODULE hModule)
{
	//1  ���һ��������
	WNDCLASS wnd;
	wnd.style = CS_VREDRAW | CS_HREDRAW;//���
	wnd.lpfnWndProc = WndProc;//���ڻص�����ָ��.
	wnd.cbClsExtra = NULL;
	wnd.cbWndExtra = NULL;
	wnd.hInstance = hModule;
	wnd.hIcon = NULL;
	wnd.hCursor = NULL;
	wnd.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wnd.lpszMenuName = NULL;
	wnd.lpszClassName = TEXT("WeChatHelper");
	//2  ע�ᴰ����
	RegisterClass(&wnd);
	//3  ��������
	HWND hWnd = CreateWindow(
		TEXT("WeChatHelper"),  //��������
		TEXT("WeChatHelper"),//������
		WS_OVERLAPPEDWINDOW,//���ڷ��
		10, 10, 500, 300, //����λ��
		NULL,             //�����ھ��
		NULL,             //�˵����
		hModule,        //ʵ�����
		NULL              //����WM_CREATE��Ϣʱ�ĸ��Ӳ���
	);
	//4  ������ʾ����
	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);
	//5  ��Ϣѭ������Ϣ�ã�
	MSG  msg = {};
	//   5.1��ȡ��Ϣ
	while (GetMessage(&msg, 0, 0, 0))
	{
		//   5.2������Ϣ
		TranslateMessage(&msg);
		//   5.3ת������Ϣ�ص�����
		DispatchMessage(&msg);
	}
}


//************************************************************
// ��������: WndProc
// ����˵��: �ص����� ���ںͿ��ƶ�ͨ�� 
// ��    ��: GuiShou
// ʱ    ��: 2019/6/30
// ��    ��: HWND hWnd,UINT Message,WPARAM wParam,LPARAM lParam
// �� �� ֵ: LRESULT 
//************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (Message == WM_COPYDATA)
	{
		COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT*)lParam;
		//����ͨ����Ϣ�ṹ��
		MessageUnion *msg = (MessageUnion*)pCopyData->lpData;
		switch (pCopyData->dwData)
		{
		//��ʾ��ά��
		case WM_ShowQrPicture:
		{
			GotoQrCode();
			HookQrCode();
		}
		break;
		//�˳�΢��
		case WM_Logout:
		{
			LogoutWeChat();
		}
		break;
		//�����ı���Ϣ
		case WM_SendTextMessage:
		{
			SendTextMessage(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
			
		}
		break;
		//�����ļ���Ϣ
		case WM_SendFileMessage:
		{
			SendFileMessage(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
		}
		break;
		//����ͼƬ��Ϣ
		case WM_SendImageMessage:
		{
			SendImageMessage(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
		}
		break;
		//��ȡ������Ϣ
		case WM_GetInformation:
		{
			GetInformation();
		}
		break;
		//����Ⱥ����
		case WM_SetRoomAnnouncement:
		{
			SetWxRoomAnnouncement(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
		}
		break;
		//ɾ������
		case WM_DeleteUser:
		{
			DeleteUser((wchar_t*)pCopyData->lpData);
		}
		break;
		//�˳�Ⱥ��
		case WM_QuitChatRoom:
		{
			QuitChatRoom((wchar_t*)pCopyData->lpData);
		}
		break;
		//����Ⱥ��Ա
		case WM_AddGroupMember:
		{
			AddGroupMember(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
		}
		break;
		//������Ƭ
		case WM_SendXmlCard:
		{
			SendXmlCard(msg->xmlcardmsg.RecverWxid, msg->xmlcardmsg.SendWxid, msg->xmlcardmsg.NickName);
		}
		break;
		//��ʾȺ��Ա
		case WM_ShowChatRoomMembers:
		{
			ShowChatRoomUser((wchar_t*)pCopyData->lpData);
		}
		break;
		//���Ӻ���
		case WM_AddUser:
		{
			AddWxUser(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
		}
		break;
		//�޸�Ⱥ����
		case WM_SetRoomName:
		{
			SetRoomName(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);

		}
		break;
		//�Զ�����
		case WM_AutoChat:
		{
			g_AutoChat = TRUE;
		}
		break;
		//ȡ���Զ�����
		case WM_CancleAutoChat:
		{
			g_AutoChat = FALSE;
		}
		break;
		//���Ͱ�����Ϣ
		case WM_SendAtMsg:
		{
			SendRoomAtMsg(msg->atmsg.chatroomid, msg->atmsg.membermsgdata1, msg->atmsg.membernickname, msg->atmsg.msgmsgdata2);
		}
		break;
		//ɾ��Ⱥ��Ա
		case WM_DelRoomMember:
		{
			DelRoomMember(msg->genericmsg.msgdata1, msg->genericmsg.msgdata2);
		}
		break;
		//��URL
		case WM_OpenUrl:
		{
			OpenUrl((wchar_t*)pCopyData->lpData);
		}
		break;
		default:
			break;
		}
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}



