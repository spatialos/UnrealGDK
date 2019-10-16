// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

namespace ExpectedFileContent
{

const char ASpatialTypeActor[] = "\
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\
// Note that this file has been generated automatically\r\n\
package unreal.generated.spatialtypeactor;\r\n\
\r\n\
import \"unreal/gdk/core_types.schema\";\r\n\
\r\n\
component SpatialTypeActor {\r\n\
	id = {{id}};\r\n\
	bool bhidden = 1;\r\n\
	bool breplicatemovement = 2;\r\n\
	bool btearoff = 3;\r\n\
	bool bcanbedamaged = 4;\r\n\
	bytes replicatedmovement = 5;\r\n\
	UnrealObjectRef attachmentreplication_attachparent = 6;\r\n\
	bytes attachmentreplication_locationoffset = 7;\r\n\
	bytes attachmentreplication_relativescale3d = 8;\r\n\
	bytes attachmentreplication_rotationoffset = 9;\r\n\
	string attachmentreplication_attachsocket = 10;\r\n\
	UnrealObjectRef attachmentreplication_attachcomponent = 11;\r\n\
	UnrealObjectRef owner = 12;\r\n\
	uint32 role = 13;\r\n\
	uint32 remoterole = 14;\r\n\
	UnrealObjectRef instigator = 15;\r\n\
}\r\n";

char ANonSpatialTypeActor[] = "\
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\
// Note that this file has been generated automatically\r\n\
package unreal.generated.nonspatialtypeactor;\r\n\
\r\n\
import \"unreal/gdk/core_types.schema\";\r\n\
\r\n\
component NonSpatialTypeActor {\r\n\
	id = {{id}};\r\n\
	bool bhidden = 1;\r\n\
	bool breplicatemovement = 2;\r\n\
	bool btearoff = 3;\r\n\
	bool bcanbedamaged = 4;\r\n\
	bytes replicatedmovement = 5;\r\n\
	UnrealObjectRef attachmentreplication_attachparent = 6;\r\n\
	bytes attachmentreplication_locationoffset = 7;\r\n\
	bytes attachmentreplication_relativescale3d = 8;\r\n\
	bytes attachmentreplication_rotationoffset = 9;\r\n\
	string attachmentreplication_attachsocket = 10;\r\n\
	UnrealObjectRef attachmentreplication_attachcomponent = 11;\r\n\
	UnrealObjectRef owner = 12;\r\n\
	uint32 role = 13;\r\n\
	uint32 remoterole = 14;\r\n\
	UnrealObjectRef instigator = 15;\r\n\
}\r\n";

const char ASpatialTypeActorComponent [] = "\
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\
// Note that this file has been generated automatically\r\n\
package unreal.generated;\r\n\
\r\n\
type SpatialTypeActorComponent {\r\n\
	bool breplicates = 1;\r\n\
	bool bisactive = 2;\r\n\
}\r\n\
\r\n\
component SpatialTypeActorComponentDynamic1 {\r\n\
	id = 10000;\r\n\
	data SpatialTypeActorComponent;\r\n\
}\r\n\
\r\n\
component SpatialTypeActorComponentDynamic2 {\r\n\
	id = 10001;\r\n\
	data SpatialTypeActorComponent;\r\n\
}\r\n\
\r\n\
component SpatialTypeActorComponentDynamic3 {\r\n\
	id = 10002;\r\n\
	data SpatialTypeActorComponent;\r\n\
}\r\n";

const char ASpatialTypeActorWithActorComponent [] = "\
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\
// Note that this file has been generated automatically\r\n\
package unreal.generated.spatialtypeactorwithactorcomponent;\r\n\
\r\n\
import \"unreal/gdk/core_types.schema\";\r\n\
\r\n\
component SpatialTypeActorWithActorComponent {\r\n\
	id = 10000;\r\n\
	bool bhidden = 1;\r\n\
	bool breplicatemovement = 2;\r\n\
	bool btearoff = 3;\r\n\
	bool bcanbedamaged = 4;\r\n\
	bytes replicatedmovement = 5;\r\n\
	UnrealObjectRef attachmentreplication_attachparent = 6;\r\n\
	bytes attachmentreplication_locationoffset = 7;\r\n\
	bytes attachmentreplication_relativescale3d = 8;\r\n\
	bytes attachmentreplication_rotationoffset = 9;\r\n\
	string attachmentreplication_attachsocket = 10;\r\n\
	UnrealObjectRef attachmentreplication_attachcomponent = 11;\r\n\
	UnrealObjectRef owner = 12;\r\n\
	uint32 role = 13;\r\n\
	uint32 remoterole = 14;\r\n\
	UnrealObjectRef instigator = 15;\r\n\
	UnrealObjectRef spatialactorcomponent = 16;\r\n\
}\r\n";

const char ASpatialTypeActorWithMultipleActorComponents [] = "\
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\
// Note that this file has been generated automatically\r\n\
package unreal.generated.spatialtypeactorwithmultipleactorcomponents;\r\n\
\r\n\
import \"unreal/gdk/core_types.schema\";\r\n\
\r\n\
component SpatialTypeActorWithMultipleActorComponents {\r\n\
	id = 10000;\r\n\
	bool bhidden = 1;\r\n\
	bool breplicatemovement = 2;\r\n\
	bool btearoff = 3;\r\n\
	bool bcanbedamaged = 4;\r\n\
	bytes replicatedmovement = 5;\r\n\
	UnrealObjectRef attachmentreplication_attachparent = 6;\r\n\
	bytes attachmentreplication_locationoffset = 7;\r\n\
	bytes attachmentreplication_relativescale3d = 8;\r\n\
	bytes attachmentreplication_rotationoffset = 9;\r\n\
	string attachmentreplication_attachsocket = 10;\r\n\
	UnrealObjectRef attachmentreplication_attachcomponent = 11;\r\n\
	UnrealObjectRef owner = 12;\r\n\
	uint32 role = 13;\r\n\
	uint32 remoterole = 14;\r\n\
	UnrealObjectRef instigator = 15;\r\n\
	UnrealObjectRef firstspatialactorcomponent = 16;\r\n\
	UnrealObjectRef secondspatialactorcomponent = 17;\r\n\
}\r\n";

const char ASpatialTypeActorWithMultipleObjectComponents [] = "\
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved\r\n\
// Note that this file has been generated automatically\r\n\
package unreal.generated.spatialtypeactorwithmultipleobjectcomponents;\r\n\
\r\n\
import \"unreal/gdk/core_types.schema\";\r\n\
\r\n\
component SpatialTypeActorWithMultipleObjectComponents {\r\n\
	id = 10000;\r\n\
	bool bhidden = 1;\r\n\
	bool breplicatemovement = 2;\r\n\
	bool btearoff = 3;\r\n\
	bool bcanbedamaged = 4;\r\n\
	bytes replicatedmovement = 5;\r\n\
	UnrealObjectRef attachmentreplication_attachparent = 6;\r\n\
	bytes attachmentreplication_locationoffset = 7;\r\n\
	bytes attachmentreplication_relativescale3d = 8;\r\n\
	bytes attachmentreplication_rotationoffset = 9;\r\n\
	string attachmentreplication_attachsocket = 10;\r\n\
	UnrealObjectRef attachmentreplication_attachcomponent = 11;\r\n\
	UnrealObjectRef owner = 12;\r\n\
	uint32 role = 13;\r\n\
	uint32 remoterole = 14;\r\n\
	UnrealObjectRef instigator = 15;\r\n\
	UnrealObjectRef firstspatialobjectcomponent = 16;\r\n\
	UnrealObjectRef secondspatialobjectcomponent = 17;\r\n\
}\r\n";

} // ExpectedFileContent

