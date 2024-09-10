# 这里会记录一些在做PA实验过程中遇到的坑，尽量让后人不要再踩坑

## 2024.09.10

今天准备做完PA0，一开始没有什么问题，直到make menuconfig这一步出了问题，
通过上网搜为什么没有make相关命令，结果一开始以为是自己没有安装linux内核，
然后开始重新安装内核，结果又卡死在解压这一步，一直显示no sun file or directory
然后我又STFW，然后又有回答说这个可能是因为没有在linux内核源代码目录下面运行make命令，
然后我就去/usr/src/目录下面但是还是需要先解压下载下来的linux-source.tar.gz压缩包，但是一直解压失败，一气之下给人家linux源代码文件夹直接全删除了，之后也没有找回来（不知道会不会对之后产生影响）
最后实在弄不好只能先去上课，结果在课上百思不得其解，突然一想到好像需要进入nemu文件夹下再运行make menuconfig，结果自己一直在ics2024/这个文件夹下面，难怪会出问题
后来回来之后又重新进行make操作，确实少安装了[libncurses5]{https://blog.csdn.net/Wangguang_/article/details/85229457}，于是进行必要的安装：
apt-get install libncurses5-dev
然后发现自己还少了bison工具，进行相应的安装之后就可以正常compile了
之后会出现一个图形化窗口，注意什么都不要做，向右进行移动然后exit退出即可

在PA0 manual的最后有提到让我们自己去STFW如何进行pdf文件的查看，查到可以用evince命令查看pdf文件，但是目前我这个ubuntu虚拟机中还没有任何pdf文档可以试，这里先记着，等回头有机会了再试一试。



现在我又遇到了一个问题，我不只是想在本地存储文件，因为我有2台电脑嘛，所以我想把代码推送到远程github仓库中去，之后我可以方便访问，结果一git remote才发现本地git仓库已经和远程仓库连接了，但是我又怕如果断开这个连接之后做PA需要访问到远程内容不方便，因此我在这里先暂且保存一下远程仓库的URL，然后再remote连接到自己的URL当中保存，之后如果需要访问NJU的仓库再重新连接也未尝不可。

edzee3000@edzee3000-VMware-Virtual-Platform:~/ICS2024/ics2024$ git remote show origin
* remote origin
  Fetch URL: git@github.com:NJU-ProjectN/ics-pa.git
  Push  URL: git@github.com:NJU-ProjectN/ics-pa.git
  HEAD branch: 2024
  Remote branches:
    2017 tracked
    2018 tracked
    2019 tracked
    2020 tracked
    2021 tracked
    2022 tracked
    2023 tracked
    2024 tracked
  Local branch configured for 'git pull':
    master merges with remote 2024

下面是一些与远程仓库进行连接的操作：
git remote add [远程仓库名] [远程仓库URL]  //与远程仓库进行连接
git remote  //显示远程仓库名称
git fetch master   //抓取远程仓库数据
git branch -a   //查看所有分支
git checkout master  //合并远程仓库到本地
git remote rm [远程仓库名]  //本地删除远程仓库





