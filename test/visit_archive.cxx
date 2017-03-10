/*
 * Copyright 2003-2017 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "tag/Tag.hxx"
#include "config/ConfigGlobal.hxx"
#include "event/Thread.hxx"
#include "input/Init.hxx"
#include "archive/ArchiveList.hxx"
#include "archive/ArchivePlugin.hxx"
#include "archive/ArchiveFile.hxx"
#include "archive/ArchiveVisitor.hxx"
#include "fs/Path.hxx"
#include "Log.hxx"

#include <stdexcept>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

class GlobalInit {
	EventThread io_thread;

public:
	GlobalInit() {
		io_thread.Start();
		config_global_init();
#ifdef ENABLE_ARCHIVE
		archive_plugin_init_all();
#endif
		input_stream_global_init(io_thread.GetEventLoop());
	}

	~GlobalInit() {
		input_stream_global_finish();
#ifdef ENABLE_ARCHIVE
		archive_plugin_deinit_all();
#endif
		config_global_finish();
	}
};

class MyArchiveVisitor final : public ArchiveVisitor {
public:
	virtual void VisitArchiveEntry(const char *path_utf8) override {
		printf("%s\n", path_utf8);
	}
};

int
main(int argc, char **argv)
try {
	if (argc != 3) {
		fprintf(stderr, "Usage: visit_archive PLUGIN PATH\n");
		return EXIT_FAILURE;
	}

	const char *plugin_name = argv[1];
	const Path path = Path::FromFS(argv[2]);

	/* initialize MPD */

	const GlobalInit init;

	/* open the archive and dump it */

	const ArchivePlugin *plugin = archive_plugin_from_name(plugin_name);
	if (plugin == nullptr) {
		fprintf(stderr, "No such plugin: %s\n", plugin_name);
		return EXIT_FAILURE;
	}

	int result = EXIT_SUCCESS;

	ArchiveFile *file = archive_file_open(plugin, path);

	MyArchiveVisitor visitor;
	file->Visit(visitor);
	file->Close();

	return result;
} catch (const std::exception &e) {
	LogError(e);
	return EXIT_FAILURE;
}
