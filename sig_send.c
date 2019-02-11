#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>

// 指定prefixを持つファイルを探す
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

// ファイル名からPIDを抽出(prefixの後ろの数字)
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
	// ----------------- pix2pix起動とスタンバイ状態へ ---------------
	char *search_dir = ".";
	char *prefix = "pix2pix_pid_";
	char finded_file[256];
	int interval = 1*100000;
	char command[256];

	// PIDファイルを削除しておく
	while(1) {
		if(find_prefix(search_dir, prefix, finded_file, 256)) {
			remove(finded_file);
		} else {
			break;
		}
	}

	// pix2pix起動（スタンバイ状態）
	pid_t my_pid = getpid();
	sprintf(command, "python sig_recv.py --parent_pid %d --prefix %s &", my_pid, prefix);
	system(command);

	// pix2pixプロセスがPIDファイルを作成するのを待つ
	while(1) {
		usleep(interval);
		if(find_prefix(search_dir, prefix, finded_file, 256)) {
			break;
		}
	}

	// pix2pixのPIDを取得
	int pix2pix_pid = get_pid(finded_file, prefix);

	// ----------------- 何らか、別の処理 ------------------
	//sleep(2);

	// ----------------- pix2pix 実行指示 ------------------
	// pix2pixへ実行指示のシグナルを送る
	printf("c -> py : SIGUSR1\n");
	kill(pix2pix_pid, SIGUSR1);

	// 完了を待つ
	int signo;
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGUSR1);
	sigprocmask(SIG_BLOCK, &ss, NULL);
	
	sigwait(&ss, &signo);
	printf("c : receive SIGUSR1\n");

	// pix2pixへ実行指示のシグナルを送る
	printf("c -> py : SIGUSR1\n");
	kill(pix2pix_pid, SIGUSR1);

	// 完了を待つ
	sigwait(&ss, &signo);
	printf("c : receive SIGUSR1\n");

	// ----------------- pix2pix スタンバイを解いて終了させる ------------------

	// pix2pixへ終了シグナルを送る
	printf("c -> py : SIGUSR2\n");
	kill(pix2pix_pid, SIGUSR2);
}
