#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool int
#define true 1
#define false 0

#define norw 8       /* �����ָ��� */
#define txmax 100    /* ���ű����� */
#define nmax 14      /* ���ֵ����λ�� */
#define al 10        /* ��ʶ������󳤶� */
#define maxerr 30    /* ������������� */
#define amax 2048    /* ��ַ�Ͻ� */
#define levmax 3     /* ����������Ƕ���������� */
#define cxmax 200    /* ��������������� */
#define stacksize 1000 /* ����ʱ����ջԪ�����Ϊ1000�� */
#define maxvar 50    /*���ı�����Ŀ,������ʽ��������Ҫ��ǰָ��*/
/* ���� */
enum symbol {
    nul, ident, number, plus, minus,
    times, slash, eql, neq,
    lss, leq, gtr, geq, lparen,
    rparen, comma, semicolon, period, becomes,
    mainsym, ifsym, thensym, whilesym,
    scansym, printsym, elsesym, endsym,
    lbrace, rbrace
};
#define symnum 29

/* ���ű��е����� */
enum object {
    constant,
    variable,
    procedure
};

/* ���������ָ�� */
enum fct {
    lit, opr, lod,
    sto, ini,cal,
    jmp, jpc,
};
#define fctnum 8

/* ���������ṹ */
struct instruction {
    enum fct f; /* ���������ָ�� */
    int l;      /* ���ò���������Ĳ�β� */
    int a;      /* ����f�Ĳ�ͬ����ͬ */
};

bool listswitch;   /* ��ʾ������������ */
bool tableswitch;  /* ��ʾ���ű���� */
char ch;           /* ��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
enum symbol sym;   /* ��ǰ�ķ��� */
char id[al + 1];   /* ��ǰident�������һ���ֽ����ڴ��0 */
int num;           /* ��ǰnumber */
int cc, ll;        /* getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�� */
int cx;            /* ���������ָ��, ȡֵ��Χ[0, cxmax-1] */
char line[81];     /* ��ȡ�л����� */
char a[al + 1];    /* ��ʱ���ţ������һ���ֽ����ڴ��0 */
struct instruction code[cxmax]; /* ����������������� */
char word[norw][al];        /* ������ */
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ���ֵ */
enum symbol ssym[256];      /* ���ַ��ķ���ֵ */
char mnemonic[fctnum][5];   /* ���������ָ������ */
bool declbegsys[symnum];    /* ��ʾ������ʼ�ķ��ż��� */
bool statbegsys[symnum];    /* ��ʾ��俪ʼ�ķ��ż��� */
bool facbegsys[symnum];     /* ��ʾ���ӿ�ʼ�ķ��ż��� */

/* ���ű�ṹ */
struct tablestruct {
    char name[al];    /* ���� */
    enum object kind; /* ���ͣ�const��var */
    int val;          /* ��ֵ����constʹ�� */
    int level;        /* �����㣬��const��ʹ�� */
    int adr;          /* ��ַ����const��ʹ�� */
    int size;           /* ��Ҫ������������ռ�, ��procedureʹ�� */
};

struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;      /* ����Դ�ļ� */
FILE* ftable;   /* ������ű� */
FILE* fcode;    /* ������������ */
FILE* foutput;  /* ����ļ�������ʾ�⣨���д������ж�Ӧ�����ɴ����׵�ַ�����޴� */
FILE* fresult;  /* ���ִ�н�� */
char fname[al];
int err;        /* ��������� */

void error(int n);
void getsym();
void getch();
void init();
void gen(enum fct x, int y, int z);
void test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
void program(int lev, int tx, bool* fsys);
void block(int lev, int tx, bool* fsys);
void stmt_list(int lev , int *ptx, int *pdx,bool* fsys);
void interpret();
void factor(bool* fsys, int* ptx, int lev);
void term(bool* fsys, int* ptx, int lev);
void condition(bool* fsys, int* ptx, int lev);
void expression(bool* fsys, int* ptx, int lev);
void statement(bool* fsys, int* ptx, int lev,int* pdx);
void listcode(int cx0);
void listall();
void vardeclaration(int* ptx, int lev, int* pdx);//��������
void constdeclaration(int* ptx, int lev, int* pdx);//���ͱ�������
void implicitconstdeclaration(int* ptx, int lev, int* pdx);//��ʽ��������
int position(char* idt,int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);

/* ������ʼ */
int main() {

    printf("Input L24 file?   ");
    scanf("%s", fname);        /* �����ļ��� */

    if ((fin = fopen(fname, "r")) == NULL) {
        printf("Can't open the input file!\n");
        exit(1);
    }

    ch = fgetc(fin);
    if (ch == EOF) {
        printf("The input file is empty!\n");
        fclose(fin);
        exit(1);
    }
    rewind(fin);

    if ((foutput = fopen("foutput.txt", "w")) == NULL) {
        printf("Can't open the output file!\n");
        exit(1);
    }

    if ((ftable = fopen("ftable.txt", "w")) == NULL) {
        printf("Can't open ftable.txt file!\n");
        exit(1);
    }

    printf("List object codes?(Y/N)");    /* �Ƿ������������� */
    scanf("%s", fname);
    listswitch = (fname[0] == 'y' || fname[0] == 'Y');

    printf("List symbol table?(Y/N)");    /* �Ƿ�������ű� */
    scanf("%s", fname);
    tableswitch = (fname[0] == 'y' || fname[0] == 'Y');

    init();        /* ��ʼ�� */
    err = 0;
    cc = ll = cx = 0;
    ch = ' ';

    bool nxtlev[symnum];
    getsym();

    addset(nxtlev, declbegsys, statbegsys, symnum);
    nxtlev[rbrace] = true;
    program(0,0,nxtlev);    /* ����ֳ��� */
    if (sym != rbrace)
    {
      error(9);  /* ȱ��������� */
    }
  
    if (err == 0) {
        printf("\n===Parsing success!===\n");
        fprintf(foutput, "\n===Parsing success!===\n");

        if ((fcode = fopen("fcode.txt", "w")) == NULL) {
            printf("Can't open fcode.txt file!\n");
            exit(1);
        }

        if ((fresult = fopen("fresult.txt", "w")) == NULL) {
            printf("Can't open fresult.txt file!\n");
            exit(1);
        }

        listall();     /* ������д��� */
        fclose(fcode);

        interpret();    /* ���ý���ִ�г��� */
        fclose(fresult);
    } else {
        printf("\n%d errors in L24 program!\n", err);
        fprintf(foutput, "\n%d errors in L24 program!\n", err);
    }

    fclose(ftable);
    fclose(foutput);
    fclose(fin);

    return 0;
}

/*
 * ��ʼ��
 */
void init() {
    int i;

    /* ���õ��ַ����� */
    for (i = 0; i <= 255; i++) {
        ssym[i] = nul;
    }
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym['='] = becomes;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym[';'] = semicolon;
    ssym['{'] = lbrace;
    ssym['}'] = rbrace;

    /* ���ñ���������,������ĸ˳�򣬱��ڶ��ֲ��� */
    strcpy(&(word[0][0]), "else");
    strcpy(&(word[1][0]), "end");
    strcpy(&(word[2][0]), "if");
    strcpy(&(word[3][0]), "main");
    strcpy(&(word[4][0]), "print");
    strcpy(&(word[5][0]), "scan");
    strcpy(&(word[6][0]), "then");
    strcpy(&(word[7][0]), "while");

    /* ���ñ����ַ��� */
    wsym[0] = elsesym;
    wsym[1] = endsym;
    wsym[2] = ifsym;
    wsym[3] = mainsym;
    wsym[4] = printsym;
    wsym[5] = scansym;
    wsym[6] = thensym;
    wsym[7] = whilesym;

    /* ����ָ������ */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[ini][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");

    /* ���÷��ż� */
    for (i = 0; i < symnum; i++) {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* ����������ʼ���ż� */
    // declbegsys[constsym] = true;
    // declbegsys[varsym] = true;
    // declbegsys[procsym] = true;

    /* ������俪ʼ���ż� */
    //statbegsys[mainsym] = true;
    statbegsys[ident] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;
    statbegsys[scansym] = true;
    statbegsys[printsym] = true;
    

    /* �������ӿ�ʼ���ż� */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[lparen] = true;
}

/*
 * ������ʵ�ּ��ϵļ�������
 */
int inset(int e, bool* s) {
    return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        sr[i] = s1[i] || s2[i];
    }
    return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        sr[i] = s1[i] && (!s2[i]);
    }
    return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        sr[i] = s1[i] && s2[i];
    }
    return 0;
}

/*
 *    ��������ӡ����λ�úʹ������
 */
void error(int n) {
    char space[81];
    memset(space, 32, 81);

    space[cc - 1] = 0; /* ����ʱ��ǰ�����Ѿ����꣬����cc-1 */

    printf("**%s^%d\n", space, n);
    fprintf(foutput, "**%s^%d\n", space, n);

    err = err + 1;
    if (err > maxerr) {
        exit(1);
    }
}

/*
 * ���˿ո񣬶�ȡһ���ַ�
 * ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
 * ������getsym����
 */
void getch() {
    if (cc == ll) { /* �жϻ��������Ƿ����ַ��������ַ����������һ���ַ����������� */
        // if (feof(fin)) {
        //     printf("Program is incomplete!\n");
        //     exit(1);
        // }
        ll = 0;
        cc = 0;
        printf("%d ", cx);
        fprintf(foutput, "%d ", cx);
        ch = ' ';
        while (ch != 10) {
            if (EOF == fscanf(fin, "%c", &ch)) {
                line[ll] = 0;
                break;
            }

            printf("%c", ch);
            fprintf(foutput, "%c", ch);
            line[ll] = ch;
            ll++;
        }
    }
    ch = line[cc];
    cc++;
}

/*
 * �ʷ���������ȡһ������
 */
void getsym() {
    int i, j, k;

    while (ch == ' ' || ch == 10 || ch == 9) {    /* ���˿ո񡢻��к��Ʊ�� */
        getch();
    }
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* ��ǰ�ĵ����Ǳ�ʶ�����Ǳ����� */ {
        k = 0;
        do {
            if (k < al) {
                a[k] = ch;
                k++;
            }
            getch();
        } while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
        a[k] = 0;
        strcpy(id, a);
        i = 0;
        j = norw - 1;
        do {    /* ������ǰ�����Ƿ�Ϊ�����֣�ʹ�ö��ַ����� */
            k = (i + j) / 2;
            if (strcmp(id, word[k]) <= 0) {
                j = k - 1;
            }
            if (strcmp(id, word[k]) >= 0) {
                i = k + 1;
            }
        } while (i <= j);
        if (i - 1 > j) /* ��ǰ�ĵ����Ǳ����� */ {
            sym = wsym[k];
        } else /* ��ǰ�ĵ����Ǳ�ʶ�� */ {
            sym = ident;
        }
    } else {
        if (ch >= '0' && ch <= '9') /* ��ǰ�ĵ��������� */ {
            k = 0;
            num = 0;
            sym = number;
            do {
                num = 10 * num + ch - '0';
                k++;
                getch();
            } while (ch >= '0' && ch <= '9'); /* ��ȡ���ֵ�ֵ */
            k--;
            if (k > nmax) /* ����λ��̫�� */ {
                error(30);
            }
        } else {
            if (ch == '=')        /* ��⸳ֵ���� */ {
                getch();
                if (ch == '=') { /* ���ȼ��ڷ��� */
                    sym = eql;
                    getch();
                } else {
                    sym = becomes;    /* ��ֵ���� */
                }
            } else {
                if (ch == '<')        /* ���С�ڻ�С�ڵ��ڷ��� */ {
                    getch();
                    if (ch == '=') {
                        sym = leq;
                        getch();
                    } else {
                        sym = lss;
                    }
                } else {
                    if (ch == '>')        /* �����ڻ���ڵ��ڷ��� */ {
                        getch();
                        if (ch == '=') {
                            sym = geq;
                            getch();
                        } else {
                            sym = gtr;
                        }
                    } else {
                      if (ch == '!') /* ��ⲻ���ڷ��� */
                      {
                        getch();
                        if (ch == '=') {
                            sym = neq;
                            getch();
                        } else {
                            sym = nul; /* ����ʶ��ķ��� */
                        }
                      } else {
                          sym = ssym[ch];        /* �����Ų�������������ʱ��ȫ�����յ��ַ����Ŵ��� */
                          if (sym != period) {
                              getch();
                          }
                      }
                    }
                }
            }
        }
    }
}

/*
 * �������������
 *
 * x: instruction.f;
 * y: instruction.l;
 * z: instruction.a;
 */
void gen(enum fct x, int y, int z) {
    if (cx >= cxmax) {
        printf("Program is too long!\n");    /* ���ɵ���������������� */
        exit(1);
    }
    if (z >= amax) {
        printf("Displacement address is too big!\n");    /* ��ַƫ��Խ�� */
        exit(1);
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    cx++;
}

/*
 * ���Ե�ǰ�����Ƿ�Ϸ�
 *
 * ���﷨�����������ںͳ��ڴ����ò��Ժ���test��
 * ��鵱ǰ���ʽ�����˳����﷨��λ�ĺϷ���
 *
 * s1:     ��Ҫ�ĵ��ʼ���
 * s2:     ���������Ҫ�ĵ��ʣ���ĳһ����״̬ʱ��
 *         �ɻָ��﷨�����������������Ĳ��䵥�ʷ��ż���
 * n:      �����
 */
void test(bool* s1, bool* s2, int n) {
    if (!inset(sym, s1)) {
        error(n);
        /* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ��� */
        while ((!inset(sym, s1)) && (!inset(sym, s2))) {
            getsym();
        }
    }
}

/*
 * �����������
 */
void program(int lev, int tx, bool* fsys) {
  int i;

	int dx;                 /* ��¼���ݷ������Ե�ַ */
	int tx0;                /* ������ʼtx */
	int cx0;                /* ������ʼcx */
	bool nxtlev[symnum];    /* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							   ���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿռ�
							   ���ݸ��¼�����*/

	dx = 3;                 /* �����ռ����ڴ�ž�̬��SL����̬��DL�ͷ��ص�ַRA  */
	tx0 = tx;		        /* ��¼�����ʶ���ĳ�ʼλ�� */
	table[tx].adr = cx;	    /* ��¼��ǰ�����Ŀ�ʼλ�� */
	gen(jmp, 0, 0);         /* ������תָ���תλ��δ֪��ʱ��0 */

	if (lev > levmax)		/* Ƕ�ײ������� */
	{
		error(32);
	}


  //�ݹ��½��﷨����
    if(sym==mainsym){
      getsym();
      if(sym==lbrace){
        getsym();

        code[table[tx0].adr].a = cx;	/* ��ǰ�����ɵ���ת������תλ�øĳɵ�ǰλ�� */
        table[tx0].adr = cx;	        /* ��¼��ǰ���̴����ַ */
        table[tx0].size = dx;	        /* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
        cx0 = cx;
        gen(ini, 0, maxvar);	            /* ����ָ���ָ��ִ��ʱ������ջ��Ϊ�����õĹ��̿���dx����Ԫ�������� */

        bool nxtlev[symnum];
        /* stmt_list��̷���Ϊ�Ҵ�����*/
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[rbrace] = true;
        stmt_list(0, &tx,&dx,nxtlev); /*����stmt_list*/
        
        gen(opr, 0, 0);	                    /* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
        memset(nxtlev, 0, sizeof(bool) * symnum);	/* �ֳ���û�в��ȼ��� */
        test(fsys, nxtlev, 8);            	/* ����̷�����ȷ�� */
        listcode(cx0);                      /* ������ֳ������ɵĴ���

        // if (sym != rbrace) {
        //     error(9); /* ȱ���Ҵ����� */
        // }
      }
      else{
        error(2);/* ȱ��������� */
      }
    }
    else {
      error(1);/* û����main��ͷ */
    }
}

/*
 * �ֳ���
 */
void stmt_list(int lev , int *ptx,int *pdx,bool* fsys)
{
  
	 bool nxtlev[symnum];    /* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							   ���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿռ�
							   ���ݸ��¼�����*/
  do
  {
    /* ����̷���Ϊ�ֺ� */
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);	/* ÿ����̷��ż��϶������ϲ��̷��ż��ϣ��Ա㲹�� */
	nxtlev[semicolon] = true;
	statement(nxtlev, ptx, lev,pdx);
  if (sym==semicolon)
  {
    getsym();//ȡsemicolon����һ���ַ����ж��Ƿ����st_m
  }
  else {
    error(88);//��������Ų��ǡ�����
    }
  } while (sym!=rbrace);
  
}

/*
 * �ڷ��ű��м���һ��
 *
 * k:      ��ʶ��������Ϊconst��var
 * ptx:    ���ű�βָ���ָ�룬Ϊ�˿��Ըı���ű�βָ���ֵ
 * lev:    ��ʶ�����ڵĲ��
 * pdx:    dxΪ��ǰӦ����ı�������Ե�ַ�������Ҫ����1
 *
 */
void enter(enum object k, int* ptx, int lev, int* pdx) {
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* ���ű��name���¼��ʶ�������� */
    table[(*ptx)].kind = k;
    switch (k) {
    case constant:    /* ���� */
        if (num > amax) {
            error(31);    /* ����Խ�� */
            num = 0;
        }
        table[(*ptx)].val = num; /* �Ǽǳ�����ֵ */
        break;
    case variable:    /* ���� */
        table[(*ptx)].level = lev;
        table[(*ptx)].adr = (*pdx);
        (*pdx)++;
        break;
    case procedure:	/* ���� */
        table[(*ptx)].level = lev;
        break;    
    }
}

/*
 * ���ұ�ʶ���ڷ��ű��е�λ�ã���tx��ʼ������ұ�ʶ��
 * �ҵ��򷵻��ڷ��ű��е�λ�ã����򷵻�0
 *
 * id:    Ҫ���ҵ�����
 * tx:    ��ǰ���ű�βָ��
 */
int position(char* id,int tx) {
    int i;
    strcpy(table[0].name, id);
    i = tx;
    while (strcmp(table[i].name, id) != 0) {
        i--;
    }
    return i;
}

/*
 * ���Ŀ������嵥
 */
void listcode(int cx0) {
    int i;
    if (listswitch) {
        printf("\n");
        for (i = cx0; i < cx; i++) {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}

/*
 * �������Ŀ�����
 */
void listall() {
    int i;
    if (listswitch) {
        for (i = 0; i < cx; i++) {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
            fprintf(fcode, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}

/*
 * ��䴦��
 */
void statement(bool* fsys, int* ptx, int lev,int* pdx) {
    int i, j,cx1, cx2;
    bool nxtlev[symnum];
    
    if (sym == ident) {    /* ׼�����ո�ֵ��䴦�� */
        j = position(id,*ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
        if (j == 0) {
            //error(11);   /* ��ʶ��δ����,�������±���*/
            implicitconstdeclaration(ptx, lev, pdx);
              j = position(id,*ptx);
              if (tableswitch)		/* ������ű� */
	{
		for (i = 1; i <= *ptx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				printf("    %d const %s ", i, table[i].name);
				printf("val=%d\n", table[i].val);
				fprintf(ftable, "    %d const %s ", i, table[i].name);
				fprintf(ftable, "val=%d\n", table[i].val);
				break;
			case variable:
				printf("    %d var   %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedure:
				printf("    %d proc  %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(ftable, "    %d proc  %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			}
		}
		printf("\n");
		fprintf(ftable, "\n");
	}

                if (sym == becomes) {
                        getsym();
                    } else {
                        error(13);    /* û�м�⵽��ֵ���� */
                    }
                memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                expression(nxtlev,ptx,lev);    /* ����ֵ�����Ҳ���ʽ */
                if (j != 0) {
                        /* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
                        gen(sto, lev - table[j].level, table[j].adr);
                    }    
        } else {
            if (table[j].kind != variable) {
                error(12);    /* ��ֵ����У���ֵ���󲿱�ʶ��Ӧ���Ǳ��� */
                j = 0;
            } else {
                getsym();
                if (sym == becomes) {
                    getsym();
                } else {
                    error(13);    /* û�м�⵽��ֵ���� */
                }
                memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                expression(nxtlev,ptx,lev);    /* ����ֵ�����Ҳ���ʽ */
                if (j != 0) {
                    /* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
                    gen(sto, lev - table[j].level, table[j].adr);
                }
            }
        }
    } 
    
    else 
    {
      if (sym == scansym) {    /* ׼������scan��䴦�� */
        getsym();
        if (sym != lparen) {
            error(34);    /* ��ʽ����Ӧ�������� */
        } else {
            do {
                getsym();
                if (sym == ident) {
                    i = position(id,*ptx);    /* ����Ҫ���ı��� */
                } else {
                    i = 0;
                }

                if (i == 0) {
                    error(35);    /* scan��������еı�ʶ��Ӧ�����������ı��� */
                } else {
                    gen(opr, 0, 16);    /* ��������ָ���ȡֵ��ջ�� */
                    gen(sto, lev - table[i].level, table[i].adr);    /* ��ջ���������������Ԫ�� */
                }
                getsym();

            } while (sym == comma);    /* һ��scan���ɶ�������� */
        }
        if (sym != rparen) {
            error(33);    /* ��ʽ����Ӧ�������� */
            while (!inset(sym, fsys))    /* �����ȣ�ֱ�������ϲ㺯���ĺ�̷��� */ {
                getsym();
            }
        } else {
            getsym();
        }
    } 
    else 
    {
      if (sym == printsym) {    /* ׼������print��䴦�� */
        getsym();
        if (sym == lparen) {
            do {
                getsym();
                memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                nxtlev[rparen] = true;
                nxtlev[comma] = true;
                expression(nxtlev, ptx,lev);    /* ���ñ��ʽ���� */
                gen(opr, 0, 14);    /* �������ָ����ջ����ֵ */
                gen(opr, 0, 15);    /* ���ɻ���ָ�� */
            } while (sym == comma);  /* һ��print��������������ֵ */
            if (sym != rparen) {
                error(33);    /* ��ʽ����Ӧ�������� */
            } else {
                getsym();
            }
        }
    } 
    else 
    {
      if (sym == ifsym) {    /* ׼������if��䴦�� */
        getsym();
        if (sym == lparen) {
            getsym();
            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
            nxtlev[rparen] = true;
            condition(nxtlev, ptx,lev); /* ������������ */
            if (sym == rparen) {
                getsym();
                if (sym == thensym) {
                    getsym();
                    if (sym == lbrace) {
                        getsym();
                        cx1 = cx;    /* ���浱ǰָ���ַ */
                        gen(jpc, 0, 0);    /* ����������תָ���ת��ַδ֪����ʱд0 */
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        nxtlev[rbrace] = true;
                        //statement(nxtlev, ptx,lev,pdx);    /* ����then������ */
                        stmt_list(lev,ptx,pdx,nxtlev);
                        code[cx1].a = cx;    /* ��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ����ʱ���л��� */
                        if (sym == rbrace) {
                            getsym();
                            if (sym == endsym)
                            {
                              getsym();
                            }
                            else if (sym == elsesym) {
                                getsym();
                                cx2 = cx;
                                gen(jmp, 0, 0);
                                code[cx1].a = cx;
                                if (sym == lbrace) {
                                    getsym();
                                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                                    nxtlev[rbrace] = true;
                                    //statement(fsys, ptx,lev,pdx);
                                    stmt_list(lev,ptx,pdx,nxtlev);
                                    code[cx2].a = cx;
                                    if (sym == rbrace) {
                                        getsym();
                                        if (sym == endsym)
                                        {
                                          getsym();
                                        }
                                        else {
                                            error(98); /*ȱ��end*/
                                            }
                                    } else {
                                        error(33);    /* ȱ���Ҵ����� */
                                    }

                                }
                            }
                            else {
                              error(99); /*ͬʱȱ��end����else*/
                            }
                        } else {
                            error(33);    /* ȱ���Ҵ����� */
                        }
                    } else {
                        error(33);    /* ȱ��������� */
                    }
                } else {
                    error(16);    /* ȱ��then */
                }
            } else {
                error(33);    /* ȱ�������� */
            }
        } else {
            error(33);    /* ȱ�������� */
        }
    } 
    else 
    {
      if (sym == whilesym) {    /* ׼������while��䴦�� */
        getsym();
        if (sym == lparen) {
            cx1 = cx;    /* �����ж�����������λ�� */
            getsym();
            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
            nxtlev[rparen] = true;
            condition(nxtlev, ptx,lev);    /* ������������ */
            cx2 = cx;    /* ����ѭ����Ľ�������һ��λ�� */
                    gen(jpc, 0, 0);    /* ����������ת��������ѭ���ĵ�ַδ֪�����Ϊ0�ȴ����� */
            if (sym == rparen) {
                getsym();
                if (sym == lbrace) {
                    getsym();
                    // cx2 = cx;    /* ����ѭ����Ľ�������һ��λ�� */
                    // gen(jpc, 0, 0);    /* ����������ת��������ѭ���ĵ�ַδ֪�����Ϊ0�ȴ����� */
                    //statement(fsys, ptx,lev,pdx);    /* ѭ���� */
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rbrace] = true;
                    stmt_list(lev,ptx,pdx,nxtlev);
                    //gen(jmp, 0, cx1);    /* ����������תָ���ת��ǰ���ж�����������λ�� */
                    //code[cx2].a = cx;    /* ��������ѭ���ĵ�ַ */
                    if (sym == rbrace) {
                        gen(jmp, 0, cx1);    /* ����������תָ���ת��ǰ���ж�����������λ�� */
                        code[cx2].a = cx;    /* ��������ѭ���ĵ�ַ */
                        getsym();
                    } else {
                        error(33);    /* ȱ���Ҵ����� */
                    }
                } else {
                    error(33);    /* ȱ��������� */
                }
            } else {
                error(33);    /* ȱ�������� */
            }
        } else {
            error(33);    /* ȱ�������� */
        }
    }
    // memset(nxtlev, 0, sizeof(bool) * symnum);    /* �������޲��ȼ��� */
    // test(fsys, nxtlev, 19);    /* �������������ȷ�� */
        }
      }
    }
  }
   memset(nxtlev, 0, sizeof(bool) * symnum);    /* �������޲��ȼ��� */
    test(fsys, nxtlev, 19);    /* �������������ȷ�� */
}

/*
 * ��ʽ������������
 */
void implicitconstdeclaration(int* ptx, int lev, int* pdx)
{
  if (sym == ident)
	{
		enter(variable, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else
	{
		error(4);	/* assign_stmt��ͷӦ�Ǳ�ʶ�� */
	}
  
}

/*
 * ������������
 */
void constdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		getsym();
		if (sym == eql || sym == becomes)
		{
			if (sym == becomes)
			{
				error(1);	/* ��=д����:= */
			}
			getsym();
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsym();
			}
			else
			{
				error(2);	/* ���������е�=��Ӧ������ */
			}
		}
		else
		{
			error(3);	/* ���������еı�ʶ����Ӧ��= */
		}
	}
	else
	{
		error(4);	/* const��Ӧ�Ǳ�ʶ�� */
	}
}

/*
 * ������������
 */
void vardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(variable, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else
	{
		error(4);	/* var����Ӧ�Ǳ�ʶ�� */
	}
}



/*
 * ���ʽ����
 */
void expression(bool* fsys, int* ptx, int lev) {
    enum symbol addop;    /* ���ڱ��������� */
    bool nxtlev[symnum];

    if (sym == plus || sym == minus) {    /* ���ʽ��ͷ�������ţ���ʱ��ǰ���ʽ������һ�����Ļ򸺵��� */
        addop = sym;    /* ���濪ͷ�������� */
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        term(nxtlev, ptx,lev);    /* ������ */
        if (addop == minus) {
            gen(opr, 0, 1);    /* �����ͷΪ��������ȡ��ָ�� */
        }
    } else {    /* ��ʱ���ʽ��������ļӼ� */
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        term(nxtlev, ptx,lev);    /* ������ */
    }
    while (sym == plus || sym == minus) {
        addop = sym;
        getsym();
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        term(nxtlev, ptx,lev);    /* ������ */
        if (addop == plus) {
            gen(opr, 0, 2);    /* ���ɼӷ�ָ�� */
        } else {
            gen(opr, 0, 3);    /* ���ɼ���ָ�� */
        }
    }
}

/*
 * ���
 */
void term(bool* fsys, int* ptx, int lev) {
    enum symbol mulop;    /* ���ڱ���˳������� */
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
    nxtlev[times] = true;
    nxtlev[slash] = true;
    factor(nxtlev, ptx,lev);    /* �������� */
    while (sym == times || sym == slash) {
        mulop = sym;
        getsym();
        factor(nxtlev, ptx,lev);
        if (mulop == times) {
            gen(opr, 0, 4);    /* ���ɳ˷�ָ�� */
        } else {
            gen(opr, 0, 5);    /* ���ɳ���ָ�� */
        }
    }
}

/*
 * ���Ӵ���
 */
void factor(bool* fsys, int* ptx, int lev) {
    int i;
    bool nxtlev[symnum];
    test(facbegsys, fsys, 24);    /* ������ӵĿ�ʼ���� */
    while (inset(sym, facbegsys))    /* ѭ���������� */ {
        if (sym == ident) {    /* ����Ϊ��������� */
            i = position(id,*ptx);    /* ���ұ�ʶ���ڷ��ű��е�λ�� */
            if (i == 0) {
                error(11);    /* ��ʶ��δ���� */
            } else {
                switch (table[i].kind) {
                case constant:    /* ��ʶ��Ϊ���� */
                    gen(lit, 0, table[i].val);    /* ֱ�Ӱѳ�����ֵ��ջ */
                    break;
                case variable:    /* ��ʶ��Ϊ���� */
                    gen(lod, lev - table[i].level, table[i].adr);    /* �ҵ�������ַ������ֵ��ջ */
                    break;
                case procedure:	/* ��ʶ��Ϊ���� */
                error(21);	/* ����Ϊ���� */
                break;    
                }
            }
            getsym();
        } else if (sym == number) {    /* ����Ϊ�� */
            if (num > amax) {
                error(31); /* ��Խ�� */
                num = 0;
            }
            gen(lit, 0, num);
            getsym();
        } else if (sym == lparen) {    /* ����Ϊ���ʽ */
            getsym();
            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
            nxtlev[rparen] = true;
            expression(nxtlev, ptx,lev);
            if (sym == rparen) {
                getsym();
            } else {
                error(22);    /* ȱ�������� */
            }
        }
        memset(nxtlev, 0, sizeof(bool) * symnum);
        nxtlev[lparen] = true;
        test(fsys, nxtlev, 23); /* һ�����Ӵ�����ϣ������ĵ���Ӧ��fsys������ */
        /* ������ǣ������ҵ���һ�����ӵĿ�ʼ��ʹ�﷨�������Լ���������ȥ */
    }
}

/*
 * ��������
 */
void condition(bool* fsys, int* ptx, int lev) {
    enum symbol relop;
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
    nxtlev[eql] = true;
    nxtlev[neq] = true;
    nxtlev[lss] = true;
    nxtlev[leq] = true;
    nxtlev[gtr] = true;
    nxtlev[geq] = true;
    expression(nxtlev, ptx,lev);
    if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq) {
        error(20); /* Ӧ��Ϊ��ϵ����� */
    } else {
        relop = sym;
        getsym();
        expression(fsys, ptx,lev);
        switch (relop) {
        case eql:
            gen(opr, 0, 8);
            break;
        case neq:
            gen(opr, 0, 9);
            break;
        case lss:
            gen(opr, 0, 10);
            break;
        case geq:
            gen(opr, 0, 11);
            break;
        case gtr:
            gen(opr, 0, 12);
            break;
        case leq:
            gen(opr, 0, 13);
            break;
        }
    }
}

/*
 * ���ͳ���
 */
void interpret() {
    int p = 0; /* ָ��ָ�� */
    int b = 1; /* ָ���ַ */
    int t = 0; /* ջ��ָ�� */
    struct instruction i;    /* ��ŵ�ǰָ�� */
    int s[stacksize];    /* ջ */

    printf("Start L24\n");
    fprintf(fresult, "Start L24\n");
    s[0] = 0; /* s[0]���� */
    s[1] = 0; /* �������������ϵ��Ԫ����Ϊ0 */
    s[2] = 0;
    s[3] = 0;
    do {
        i = code[p];    /* ����ǰָ�� */
        p = p + 1;
        switch (i.f) {
        case lit:    /* ������a��ֵȡ��ջ�� */
            t = t + 1;
            s[t] = i.a;
            break;
        case opr:    /* ��ѧ���߼����� */
            switch (i.a) {
            case 0:  /* �������ý����󷵻� */
                t = b - 1;
                p = s[t + 3];
                b = s[t + 2];
                break;
            case 1: /* ջ��Ԫ��ȡ�� */
                s[t] = -s[t];
                break;
            case 2: /* ��ջ�������ջ���������ջԪ�أ����ֵ��ջ */
                t = t - 1;
                s[t] = s[t] + s[t + 1];
                break;
            case 3:/* ��ջ�����ȥջ���� */
                t = t - 1;
                s[t] = s[t] - s[t + 1];
                break;
            case 4:/* ��ջ�������ջ���� */
                t = t - 1;
                s[t] = s[t] * s[t + 1];
                break;
            case 5:/* ��ջ�������ջ���� */
                t = t - 1;
                s[t] = s[t] / s[t + 1];
                break;
            case 6:/* ջ��Ԫ�ص���ż�ж� */
                s[t] = s[t] % 2;
                break;
            case 8:/* ��ջ������ջ�����Ƿ���� */
                t = t - 1;
                s[t] = (s[t] == s[t + 1]);
                break;
            case 9:/* ��ջ������ջ�����Ƿ񲻵� */
                t = t - 1;
                s[t] = (s[t] != s[t + 1]);
                break;
            case 10:/* ��ջ�����Ƿ�С��ջ���� */
                t = t - 1;
                s[t] = (s[t] < s[t + 1]);
                break;
            case 11:/* ��ջ�����Ƿ���ڵ���ջ���� */
                t = t - 1;
                s[t] = (s[t] >= s[t + 1]);
                break;
            case 12:/* ��ջ�����Ƿ����ջ���� */
                t = t - 1;
                s[t] = (s[t] > s[t + 1]);
                break;
            case 13: /* ��ջ�����Ƿ�С�ڵ���ջ���� */
                t = t - 1;
                s[t] = (s[t] <= s[t + 1]);
                break;
            case 14:/* ջ��ֵ��� */
                printf("%d", s[t]);
                fprintf(fresult, "%d", s[t]);
                t = t - 1;
                break;
            case 15:/* ������з� */
                printf("\n");
                fprintf(fresult, "\n");
                break;
            case 16:/* ����һ����������ջ�� */
                t = t + 1;
                printf("?");
                fprintf(fresult, "?");
                scanf("%d", &(s[t]));
                fprintf(fresult, "%d\n", s[t]);
                break;
            }
            break;
        case lod:    /* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
            t = t + 1;
            s[t] = s[base(i.l, s, b) + i.a];
            break;
        case sto:    /* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
            s[base(i.l, s, b) + i.a] = s[t];
            t = t - 1;
            break;
        // case cal:	/* �����ӹ��� */
        //     s[t + 1] = base(i.l, s, b);	/* �������̻���ַ��ջ����������̬�� */
        //     s[t + 2] = b;	/* �������̻���ַ��ջ����������̬�� */
        //     s[t + 3] = p;	/* ����ǰָ��ָ����ջ�������淵�ص�ַ */
        //     b = t + 1;	/* �ı����ַָ��ֵΪ�¹��̵Ļ���ַ */
        //     p = i.a;	/* ��ת */
        //     break;    
        case ini:    /* ������ջ��Ϊ�����õĹ��̿���a����Ԫ�������� */
            t = t + i.a;
            break;
        case jmp:    /* ֱ����ת */
            p = i.a;
            break;
        case jpc:    /* ������ת */
            if (s[t] == 0)
                p = i.a;
            t = t - 1;
            break;
        }
    } while (p != 0);
    printf("End L24\n");
    fprintf(fresult, "End L24\n");
}

/* ͨ�����̻�ַ����l����̵Ļ�ַ */
int base(int l, int* s, int b) {
    int b1;
    b1 = b;
    while (l > 0) {
        b1 = s[b1];
        l--;
    }
    return b1;
}

