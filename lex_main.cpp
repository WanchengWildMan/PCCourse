// Lexical_Analysis.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "iostream"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
using namespace std;
//�ʷ���������
//���ȶ����ֱ���
/*
��һ�ࣺ��ʶ��   letter(letter | digit)*  ���
�ڶ��ࣺ����    (digit)+  ���
�����ࣺ������(32)
auto       break    case     char        const      continue
default    do       double   else        enum       extern
float      for      goto     if          int        long
register   return   short    signed      sizeof     static
struct     switch   typedef  union       unsigned   void
volatile    while

�����ࣺ���  ��/*������//���� () { } [ ] " "  '
�����ࣺ����� <��<=��>��>=��=��+��-��*��/��^��

�����п������Ž��б��룺
<$,0>
<auto,1>
...
<while,32>
<+��33>
<-,34>
<*,35>
</,36>
<<,37>
<<=,38>
<>,39>
<>=,40>
<=,41>
<==,42>
<!=,43>
<;,44>
<(,45>
<),46>
<^,47>
<,,48>
<",49>
<',50>
<#,51>
<&,52>
<&&,53>
<|,54>
<||,55>
<%,56>
<~,57>
<<<,58>����
<>>,59>����
<[,60>
<],61>
<{,62>
<},63>
<\,64>
<.,65>
<?,66>
<:,67>
<!,68>
"[","]","{","}"
<����99  ,��ֵ>
<��ʶ��100 ����ʶ��ָ��>


*/

/****************************************************************************************/
//ȫ�ֱ����������ֱ�
static char reserveWord[32][20] = {
    "auto",     "break",  "case",    "char",   "const",    "continue",
    "default",  "do",     "double",  "else",   "enum",     "extern",
    "float",    "for",    "goto",    "if",     "int",      "long",
    "register", "return", "short",   "signed", "sizeof",   "static",
    "struct",   "switch", "typedef", "union",  "unsigned", "void",
    "volatile", "while"};
//����������,������Ҫ������������
static char operatorOrDelimiter[41][10] = {
    "+",  "-", "*", "/",       "<",  "<=", ">",  ">=", "=", "==", "!=",
    ";",  "(", ")", "^",       ",",  "\"", "\'", "#",  "&", "&&", "|",
    "||", "%", "~", "<<",      ">>", "[",  "]",  "{",  "}", "\\", ".",
    "\?", ":", "!", /**/ "+=", "-=", "*=", "|=", "&="};

static char IDentifierTbl[1000][50] = {""};  //��ʶ����
int IDentifierTblLen = 0;
int lineMap[1000004], colMap[1000004];
/****************************************************************************************/

/********�����Ƿ��Ǳ�����*****************/
int searchReserve(char reserveWord[][20], char s[]) {
  //�Լ�ʵ��
  for (int i = 0; i < 32; i++) {
    if (strcmp(reserveWord[i], s) == 0) {
      return i;
    }
  }
  return -1;
}
/********���ұ�����*****************/

/*********************�ж��Ƿ�Ϊ��ĸ********************/
bool IsLetter(char letter) {
  //ע��C���������»���ҲΪ��ʶ����һ���ֿ��Է����ײ��������ط�
  if (letter >= 'a' && letter <= 'z' || letter >= 'A' && letter <= 'Z' ||
      letter == '_') {
    return true;
  } else {
    return false;
  }
}
/*********************�ж��Ƿ�Ϊ��ĸ********************/

/*****************�ж��Ƿ�Ϊ����************************/
bool IsDigit(char digit) {
  if (digit >= '0' && digit <= '9') {
    return true;
  } else {
    return false;
  }
}
/*****************�ж��Ƿ�Ϊ����************************/
bool IsOpr(char *s) {
  for (int i = 0; i < 36; i++) {
    if (strcpy(operatorOrDelimiter[i], s) == 0) return 1;
  }
  return 0;
}
/************�ж��ǲ���hex�Ϸ�*********************/
bool IsHexOK(char ch) {
  return IsDigit(ch) || (tolower(ch) >= 'a' && tolower(ch) <= 'f');
}
/***********�ж�bin�ǲ��ǺϷ�*********************/
bool IsBinOK(char ch) { return ch == '0' || ch == '1'; }
bool IsOctOK(char ch) { return '0' <= ch && ch <= '7'; }
/********************����Ԥ����ȡ�����õ��ַ���ע��**********************/
int lineCnt = 0;
void filterResource(char r[], int pProject) {
  char tempString[10000];
  int count = 0, colCnt = 0;
  for (int i = 0; i <= pProject; i++) {
    //�Լ�ʵ�֣���Դ������ɾ�����з�\n���Ʊ��\t���س���\r��ɾ��/*  */����
    ////������ע�͡�
    if (r[i] == '\n' || r[i] == '\t' || r[i] == '\r') {
      if (r[i] == '\n') lineCnt++, colCnt = 0;
      continue;
    } else if (r[i] == '/' && r[i + 1] == '*') {
      int j = i + 2;
      while (!(r[j] == '*' && r[j + 1] == '/')) j++;
    } else if (r[i] == '/' && r[i + 1] == '/') {
      int j = i + 1;
      while (r[j + 1] != '\n') j++;
      i = j;
    } else
      tempString[count++] = r[i], lineMap[count - 1] = lineCnt,
      colMap[count - 1] = ++colCnt;
  }
  tempString[count] = '\0';
  strcpy(r, tempString);  //��������֮���Դ����
}
/********************����Ԥ����ȡ�����õ��ַ���ע��**********************/

/****************************�����ӳ����㷨����***********************/
void GetToken(int &syn, char resourceProject[], char token[], int &pProject) {
  try {
    //����DFA��״̬ת��ͼ���
    int i, count = 0;  // count������token[]��ָʾ�����ռ������ַ�
    char ch;           //��Ϊ�ж�ʹ��
    ch = resourceProject[pProject];
    while (ch == ' ') {
      //���˿ո񣬷�ֹ������ʶ���˿ո������
      pProject++;
      ch = resourceProject[pProject];
    }
    for (i = 0; i < 20; i++) {
      //ÿ���ռ�ǰ������
      token[i] = '\0';
    }
    if (IsLetter(resourceProject[pProject])) {
      //��ͷΪ��ĸ�����������ַ����ж��Ƿ�Ϊ��ʶ���������ַ����洢��token�У��Լ�ʵ�֡�
      while (IsLetter(ch)) {
        token[count++] = ch, ch = resourceProject[pProject + count],
        token[count] = '\0';
      }

      //����ҵ��ؼ��ֵ��ֱ��룬�Լ�ʵ��
      syn = searchReserve(reserveWord, token);

      if (syn == -1) {
        //�����ǹؼ������Ǳ�ʶ��
        syn = 100;  //��ʶ���ֱ���
      }
      pProject += count;
      return;
    } else if (IsDigit(resourceProject[pProject])) {  //-----------ʶ����
      // if (resourceProject[pProject] == '-')
      //   token[count++] = resourceProject[pProject++];
      if (resourceProject[pProject] == '0' &&
          IsLetter(resourceProject[pProject + 1])) {  // bin��hex
        if (resourceProject[pProject + 1] == 'x' ||   // hex
            resourceProject[pProject + 1] == 'X') {
          strcpy(token + count, "0x"), count += 2, pProject += 2;
          while (IsDigit(resourceProject[pProject])) {
            if (!IsHexOK(resourceProject[pProject])) {
              char msg[1004];
              sprintf(msg, "Hex Error:%d,%d", lineMap[pProject],
                      colMap[pProject]);
              throw msg;
            }
            token[count++] = resourceProject[pProject], pProject++;
          }
        } else if (resourceProject[pProject + 1] == 'B' ||
                   resourceProject[pProject + 1] == 'b') {  // bin
          strcpy(token + count, "0b"), count += 2, pProject += 2;
          while (IsDigit(resourceProject[pProject])) {
            if (!IsBinOK(resourceProject[pProject])) {
              char msg[1004];
              sprintf(msg, "Bin Error:%d,%d", lineMap[pProject],
                      colMap[pProject]);
              throw msg;
            }
            token[count++] = resourceProject[pProject], pProject++;
          }
        }
      } else if (resourceProject[pProject] == '0' &&
                 IsDigit(resourceProject[pProject + 1])) {  // Oct
        while (IsDigit(resourceProject[pProject])) {
          if (!IsOctOK(resourceProject[pProject])) {
            char msg[1004];
            sprintf(msg, "Oct Error:%d,%d", lineMap[pProject],
                    colMap[pProject]);
            throw msg;
          }
          token[count++] = resourceProject[pProject], pProject++;
        }
      } else {  //С��������
        //���ַ�Ϊ����
        while (IsDigit(resourceProject[pProject])) {  // dec
          //�������
          token[count++] = resourceProject[pProject], pProject++;
        }  //�����һ���ַ������´ν�Ҫ��ʼ��ָ��λ��
        if (resourceProject[pProject] == '.') {
          token[count++] = resourceProject[pProject++];
          while (IsDigit(resourceProject[pProject])) {
            token[count++] = resourceProject[pProject], pProject++;
          }
          if (tolower(resourceProject[pProject]) == 'e') {
            while (IsDigit(resourceProject[pProject])) {
              token[count++] = resourceProject[pProject++];
            }
          }
          char _[2];
          _[0] = resourceProject[pProject], _[1] = '\0';
          if (IsLetter(resourceProject[pProject]) ||
              resourceProject[pProject] == '.' || !IsOpr(_)) {
            char msg[1004];
            sprintf(msg, "Dec Error:%d,%d", lineMap[pProject],
                    colMap[pProject]);
            throw msg;
          }
        }
      }
      token[count] = '\0';
      syn = 99;  //�����ֱ���
    } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ';' ||
               ch == '(' || ch == ')' || ch == '^' || ch == ',' || ch == '\"' ||
               ch == '\'' || ch == '~' || ch == '#' || ch == '%' || ch == '[' ||
               ch == ']' || ch == '{' || ch == '}' || ch == '\\' || ch == '.' ||
               ch == '\?' || ch == ':') {
      //��Ϊ��������߽�������õ����
      token[0] = resourceProject[pProject];
      token[1] = '\0';  //�γɵ��ַ���
      for (i = 0; i < 36; i++) {
        //������������
        if (strcmp(token, operatorOrDelimiter[i]) == 0) {
          syn = 33 + i;  //����ֱ��룬ʹ����һ�㼼�ɣ�ʹ֮������ӳ��
          break;  //�鵽���Ƴ�
        }
      }

      pProject++;  //ָ�����ƣ�Ϊ��һɨ����׼��
      return;
    } else if (resourceProject[pProject] == '<') {
      //<,<=,<<
      pProject++;  //���ƣ���ǰ����
      if (resourceProject[pProject] == '=') {
        syn = 38;
      } else if (resourceProject[pProject] == '<') {
        //����
        pProject--;
        syn = 58;
      } else {
        pProject--;
        syn = 37;
      }
      pProject++;  //ָ������
      return;
    } else if (resourceProject[pProject] == '>') {
      //>,>=,>>
      pProject++;
      if (resourceProject[pProject] == '=') {
        syn = 40;
      } else if (resourceProject[pProject] == '>') {
        syn = 59;
      } else {
        pProject--;
        syn = 39;
      }
      pProject++;
      return;
    } else if (resourceProject[pProject] == '=') {
      //=.==
      pProject++;
      if (resourceProject[pProject] == '=') {
        syn = 42;
      } else {
        pProject--;
        syn = 41;
      }
      pProject++;
      return;
    } else if (resourceProject[pProject] == '!') {
      //!,!=
      pProject++;
      if (resourceProject[pProject] == '=') {
        syn = 43;
      } else {
        syn = 68;
        pProject--;
      }
      pProject++;
      return;
    } else if (resourceProject[pProject] == '&') {
      //&,&&
      pProject++;
      if (resourceProject[pProject] == '&') {
        syn = 53;
      } else {
        pProject--;
        syn = 52;
      }
      pProject++;
      return;
    } else if (resourceProject[pProject] == '|') {
      //|,||
      pProject++;
      if (resourceProject[pProject] == '|') {
        syn = 55;
      } else {
        pProject--;
        syn = 54;
      }
      pProject++;
      return;
    }  else if (resourceProject[pProject] == '$') {
      //������
      syn = 0;  //�ֱ���Ϊ0
    } else {
      //���ܱ����ϴʷ�����ʶ�������
      printf("error��there is no exist %c \n", ch);
      exit(0);
    }
  } catch (char *e) {
    cout << e;
    exit(1);
  }
}

int main() {
  //��һ���ļ�����ȡ���е�Դ����
  char resourceProject[10000];
  char token[20] = {0};
  int syn = -1, i;   //��ʼ��
  int pProject = 0;  //Դ����ָ��
  FILE *fp, *fp1;
  if ((fp = fopen("D:\\test.c", "r")) == NULL) {
    //��Դ����
    cout << "can't open this file";
    exit(0);
  }
  resourceProject[pProject] = fgetc(fp);  //��ȡһ���ַ�
  while (resourceProject[pProject] != EOF) {
    //��Դ�������resourceProject[]����
    pProject++;
    resourceProject[pProject] = fgetc(fp);
  }
  resourceProject[++pProject] = '\0';
  fclose(fp);
  cout << endl << "Դ����Ϊ:" << endl;
  cout << resourceProject << endl;
  //��Դ������й���
  filterResource(resourceProject, pProject);
  cout << endl << "����֮��ĳ���:" << endl;
  cout << resourceProject << endl;
  pProject = 0;  //��ͷ��ʼ��

  if ((fp1 = fopen("D:\\test_compile.txt", "w+")) == NULL) {
    //��Դ����
    cout << "can't open this file";
    exit(0);
  }
  while (syn != 0) {
    syn = -1;
    //����ɨ��
    GetToken(syn, resourceProject, token, pProject);
    if (syn == 100) {
      //�ж��Ƿ������б�ʶ�����У��Լ�ʵ��
      int ok = 0;
      for (int i = 0; i < 100; i++)
        if (strcmp(IDentifierTbl[i], token) == 0) {
          ok = 1;
          break;
        }
      if (!ok) strcpy(IDentifierTbl[IDentifierTblLen++], token);

      printf("(��ʶ��  ,%s)\n", token);
      fprintf(fp1, "(��ʶ��   ,%s)\n", token);
    } else if (syn >= 1 && syn <= 32) {
      //������
      printf("(%s   ,  --)\n", reserveWord[syn - 1]);
      fprintf(fp1, "(%s   ,  --)\n", reserveWord[syn - 1]);
    } else if (syn == 99) {
      // const ����
      printf("(����   ,   %s)\n", token);
      fprintf(fp1, "(����   ,   %s)\n", token);
    } else if (syn >= 33 && syn <= 68) {
      printf("(%s   ,   --)\n", operatorOrDelimiter[syn - 33]);
      fprintf(fp1, "(%s   ,   --)\n", operatorOrDelimiter[syn - 33]);
    }
  }
  for (i = 0; i < 100; i++) {
    //�����ʶ������
    printf("��%d����ʶ����  %s\n", i + 1, IDentifierTbl[i]);
    fprintf(fp1, "��%d����ʶ����  %s\n", i + 1, IDentifierTbl[i]);
  }
  fclose(fp1);
  return 0;
}
