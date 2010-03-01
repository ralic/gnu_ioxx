# ioxx/cfg.mk

gnu_rel_host    := dl.sv.nongnu.org
upload_dest_dir_:= /releases/ioxx/
old_NEWS_hash   := 34f3713a9bc8d6c450f18f611f3e78ad
gpg_key_ID      := 99089D72
url_dir_list    := http://download.savannah.nongnu.org/releases/ioxx
today           := $(date "+%Y-%m-%d")
TAR_OPTIONS     += --mtime=$(today)
