#include <stdio.h>
typedef struct StudentStruct{
	char * initial;
	int num;
}Student;
void swap(Student *s1, Student* s2){
	Student s3,s4;
	s3 = *s1;
	s4 = *s2;
	s1 -> initial = s4.initial;
	s2 -> initial = s3.initial;
	s1 -> num = s4.num;
	s2 -> num = s3.num;
	printf("swap Function call!!\n");
}
int main(){
	Student s1, s2;

	s1.initial = "KimSeunghyeok";
	s1.num = 202002473;

	s2.initial = "programing";
	s2.num = 54321;
	printf("myname : %s\nmynumber : %d\nothername : %s\nclass : %d\n",s1.initial, s1.num, s2.initial, s2.num);

	printf("[before] myname : %s, othername : %s\n",s1.initial, s2.initial);
	printf("[before] mynum : %d, othernum : %d\n", s1.num, s2.num);

	swap(&s1, &s2);
	
	printf("[after] myname : %s, othername : %s\n",s1.initial, s2.initial);
	printf("[after] mynum : %d, othernum : %d\n", s1.num, s2.num);

	return 0;
}
