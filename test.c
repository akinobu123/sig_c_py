#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>

// �w��prefix�����t�@�C����T��
bool find_prefix(
	char *search_dir, 
	char *file_prefix, 
	char *out_file /* out */, 
	int out_file_max)
{
	DIR *dir;
	struct dirent *dp;

	bool finded = false;
	dir = opendir(search_dir);
	for(dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
		if(strncmp(dp->d_name, file_prefix, strlen(file_prefix)) == 0) {
			strncpy(out_file, dp->d_name, out_file_max);
			out_file[out_file_max-1] = '\0';
			finded = true;
		}
	}
	closedir(dir);

	return finded;
}

// �t�@�C��������PID�𒊏o(prefix�̌��̐���)
int get_pid(char *filename, char *prefix)
{
	char buf[256];
	int filename_len = strlen(filename);
	int prefix_len = strlen(prefix);
	strncpy(buf, filename + prefix_len, filename_len - prefix_len);
	int pid = atoi(buf);
	return pid;
}

int main()
{
	// ----------------- pix2pix�N���ƃX�^���o�C��Ԃ� ---------------
	char *search_dir = ".";
	char *prefix = "pix2pix_pid_";
	char finded_file[256];
	int interval = 1*100000;
	char command[256];

	// PID�t�@�C�����폜���Ă���
	while(1) {
		if(find_prefix(search_dir, prefix, finded_file, 256)) {
			remove(finded_file);
		} else {
			break;
		}
	}

	// pix2pix�N���i�X�^���o�C��ԁj
	pid_t my_pid = getpid();
	sprintf(command, "python test.py --parent_pid %d --prefix %s &", my_pid, prefix);
	system(command);

	// pix2pix�v���Z�X��PID�t�@�C�����쐬����̂�҂�
	while(1) {
		usleep(interval);
		if(find_prefix(search_dir, prefix, finded_file, 256)) {
			break;
		}
	}

	// pix2pix��PID���擾
	int pix2pix_pid = get_pid(finded_file, prefix);

	// ----------------- ���炩�A�ʂ̏��� ------------------
	sleep(2);

	// ----------------- pix2pix ���s�w�� ------------------
	// pix2pix�֎��s�w���̃V�O�i���𑗂�
	printf("c -> py : SIGUSR1\n");
	kill(pix2pix_pid, SIGUSR1);

	// ������҂�
	int signo;
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGUSR1);
	sigprocmask(SIG_BLOCK, &ss, NULL);
	
	sigwait(&ss, &signo);
	printf("c : receive SIGUSR1\n");

	// pix2pix�֎��s�w���̃V�O�i���𑗂�
	printf("c -> py : SIGUSR1\n");
	kill(pix2pix_pid, SIGUSR1);

	// ������҂�
	sigwait(&ss, &signo);
	printf("c : receive SIGUSR1\n");

	// ----------------- pix2pix �X�^���o�C�������ďI�������� ------------------

	// pix2pix�֏I���V�O�i���𑗂�
	printf("c -> py : SIGUSR2\n");
	kill(pix2pix_pid, SIGUSR2);
}
