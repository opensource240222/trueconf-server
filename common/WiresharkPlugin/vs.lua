local bit32 = bit32 or require("bit32")

local function debug(...)
	--print(...)
end

local vs = Proto("vs", "Visicron protocol")

vs.fields.hs = ProtoField.new("Handshake", "vs.hs", ftypes.BYTES)
-- Handshake header (net::HandshakeHeader)
vs.fields.hs_primary = ProtoField.new("Primary field", "vs.hs.primary", ftypes.STRING)
vs.fields.hs_version = ProtoField.new("Version", "vs.hs.version", ftypes.UINT32, nil, base.DEC, 0x0000001f)
vs.fields.hs_head_cksum = ProtoField.new("Header checksum", "vs.hs.head_cksum", ftypes.UINT32, nil, base.DEC, 0x00001fe0)
vs.fields.hs_body_cksum = ProtoField.new("Body checksum", "vs.hs.body_cksum", ftypes.UINT32, nil, base.DEC, 0x0007e000)
vs.fields.hs_body_length = ProtoField.new("Body length minus one", "vs.hs.body_length", ftypes.UINT32, nil, base.DEC, 0xfff80000)
vs.experts.hs_version_unk = ProtoExpert.new("vs.hs.version_unk", "Unknown version", expert.group.UNDECODED, expert.severity.NOTE)
vs.experts.hs_head_cksum_bad = ProtoExpert.new("vs.hs.head_cksum_bad", "Header checksum doesn't match", expert.group.CHECKSUM, expert.severity.ERROR)
vs.experts.hs_body_cksum_bad = ProtoExpert.new("vs.hs.body_cksum_bad", "Body checksum doesn't match", expert.group.CHECKSUM, expert.severity.ERROR)
vs.experts.hs_body_unk = ProtoExpert.new("vs.hs.body_unk", "Undecoded body part", expert.group.UNDECODED, expert.severity.NOTE)
vs.fields.hs_real_head_cksum = ProtoField.new("Calculated header checksum", "vs.hs.real_head_cksum", ftypes.UINT8)
vs.fields.hs_real_body_cksum = ProtoField.new("Calculated body checksum", "vs.hs.real_body_cksum", ftypes.UINT8)
-- Stream handshake body
local hs_client_type_desc = {
	[0] = "sender",
	[1] = "receiver",
}
vs.fields.hs_cl_type = ProtoField.new("Stream client type", "vs.hs.cl_type", ftypes.UINT8, hs_client_type_desc)
vs.fields.hs_conf = ProtoField.new("Conference name", "vs.hs.conf", ftypes.STRINGZ)
vs.fields.hs_part = ProtoField.new("Participant name", "vs.hs.part", ftypes.STRINGZ)
vs.fields.hs_conn_part = ProtoField.new("Connected participant name", "vs.hs.conn_part", ftypes.STRINGZ)
vs.fields.hs_ep = ProtoField.new("Endpoint name", "vs.hs.ep", ftypes.STRINGZ)
vs.fields.hs_ntracks = ProtoField.new("Track count", "vs.hs.ntracks", ftypes.UINT8)
vs.fields.hs_tracks = ProtoField.new("Tracks", "vs.hs.tracks", ftypes.BYTES)
vs.experts.hs_cl_type_bad = ProtoExpert.new("vs.hs.cl_type_unk", "Wrong stream client type", expert.group.MALFORMED, expert.severity.WARN)
-- Frame header (stream::FrameHeader)
vs.fields.f = ProtoField.new("Frame", "vs.f", ftypes.BYTES)
vs.fields.f_length = ProtoField.new("Payload length", "vs.f.length", ftypes.UINT16)
vs.fields.f_ticks = ProtoField.new("Tick count", "vs.f.ticks", ftypes.UINT32)
local track_desc = {
	[1] = "audio",
	[2] = "video",
	[5] = "data",
	[128] = "garbage",
	[254] = "command",
	[255] = "command (old)",
}
vs.fields.f_track = ProtoField.new("Track", "vs.f.track", ftypes.UINT8, track_desc)
vs.fields.f_cksum = ProtoField.new("Checksum", "vs.f.cksum", ftypes.UINT8)
vs.experts.f_track_unk = ProtoExpert.new("vs.f.track_unk", "Unknown track", expert.group.UNDECODED, expert.severity.NOTE)
vs.experts.f_cksum_bad = ProtoExpert.new("vs.f.cksum_bad", "Checksum doesn't match", expert.group.CHECKSUM, expert.severity.ERROR)
vs.fields.f_real_cksum = ProtoField.new("Calculated checksum", "vs.f.real_cksum", ftypes.UINT8)
vs.fields.f_data = ProtoField.new("Codec data", "vs.f.data", ftypes.BYTES)
-- Video frame metadata
vs.fields.f_key = ProtoField.new("Key frame", "vs.f.key", ftypes.BOOLEAN)
vs.fields.f_duration = ProtoField.new("Frame duration (milliseconds)", "vs.f.duration", ftypes.UINT32)
-- Video slice header (stream::SliceHeader)
vs.fields.f_vh_slice = ProtoField.new("Slice ID", "vs.f.vh.slice", ftypes.UINT8)
vs.fields.f_vh_slice_max = ProtoField.new("First slice ID", "vs.f.vh.slice_max", ftypes.UINT8)
vs.fields.f_vh_frame_cnt = ProtoField.new("Frame counter", "vs.f.vh.frame_cnt", ftypes.UINT8)
vs.experts.f_vh_slice_bad = ProtoExpert.new("vs.f.vh.slice_bad", "Slice ID > First Slice ID", expert.group.MALFORMED, expert.severity.WARN)
vs.experts.f_slice_missed = ProtoExpert.new("vs.f.slice_missed", "Slice missed", expert.group.SEQUENCE, expert.severity.WARN)
vs.experts.f_slice_missed_pf = ProtoExpert.new("vs.f.slice_missed_pf", "Slice missed in previous frame", expert.group.SEQUENCE, expert.severity.WARN)
vs.experts.f_frame_missed = ProtoExpert.new("vs.f.frame_missed", "Frame missed", expert.group.SEQUENCE, expert.severity.WARN)
-- Video SVC header (stream::SVCHeader)
vs.fields.svc = ProtoField.new("SVC header", "vs.svc", ftypes.UINT8, nil, base.BIN)
vs.fields.svc_maxspatial = ProtoField.new("Max spatial", "vs.svc.maxspatial", ftypes.UINT8, nil, base.DEC, 0x03)
vs.fields.svc_temporal = ProtoField.new("Temporal", "vs.svc.temporal", ftypes.UINT8, nil, base.DEC, 0x0c)
vs.fields.svc_spatial = ProtoField.new("Spatial", "vs.svc.spatial", ftypes.UINT8, nil, base.DEC, 0x30)
vs.fields.svc_quality = ProtoField.new("Quality", "vs.svc.quality", ftypes.UINT8, nil, base.DEC, 0xc0)
-- stream::Command
vs.fields.cmd = ProtoField.new("Command", "vs.cmd", ftypes.BYTES)
local cmd_type_desc = {
	[0] = "empty",
	[1] = "key frame request",
	[2] = "restrict bitrate",
	[3] = "set FPS vs quality",
	[4] = "change send media format",
	[5] = "change receive media format",
	[6] = "statistics",
	[7] = "request packet",
	[8] = "time delay",
	[9] = "ping",
	[10] = "broker statistics",
	[11] = "restrict bitrate (SVC)",
}
vs.fields.cmd_type = ProtoField.new("Type", "vs.cmd.type", ftypes.UINT8, cmd_type_desc)
local cmd_subtype_desc = {
	[1] = "request",
	[2] = "reply",
	[3] = "ack",
	[4] = "info",
}
vs.fields.cmd_subtype = ProtoField.new("Sub type", "vs.cmd.subtype", ftypes.UINT8, cmd_subtype_desc)
local cmd_ret_desc = {
	[0] = "ok",
	[255] = "error",
}
vs.fields.cmd_ret = ProtoField.new("Return code", "vs.cmd.ret", ftypes.UINT8, cmd_ret_desc)
vs.fields.cmd_data_length = ProtoField.new("Data length", "vs.cmd.data_length", ftypes.UINT8)
vs.fields.cmd_data = ProtoField.new("Command data", "vs.cmd.data", ftypes.BYTES)
vs.fields.cmd_bitrate = ProtoField.new("Bitrate", "vs.cmd.bitrate", ftypes.UINT32)
vs.fields.cmd_vbitrate = ProtoField.new("Video bitrate", "vs.cmd.vbitrate", ftypes.UINT32)
vs.experts.cmd_data_unk = ProtoExpert.new("vs.cmd.data_unk", "Undecoded command data", expert.group.UNDECODED, expert.severity.NOTE)
vs.experts.cmd_truncated = ProtoExpert.new("vs.cmd.truncated", "Command truncated", expert.group.MALFORMED, expert.severity.ERROR)

-- VS_MediaFormat
vs.fields.mf = ProtoField.new("Media format", "vs.mf", ftypes.BYTES)
vs.fields.mf_length = ProtoField.new("Length", "vs.mf.length", ftypes.UINT32)
vs.fields.mf_vheigth = ProtoField.new("Video height", "vs.mf.vheight", ftypes.UINT32)
vs.fields.mf_vwidth = ProtoField.new("Video width", "vs.mf.vwidth", ftypes.UINT32)
local mf_vcodec_desc = {
	[0x31363268] = "H.261",
	[0x33363268] = "H.263",
	[0x70363268] = "H.263+",
	[0x34366876] = "H.264",
	[0x30387670] = "VP8",
	[0x30397076] = "VP9",
	[0x64687670] = "VP8 HD",
	[0x74737670] = "VP8 Stereo",
	[0x32306378] = "Cyclone",
}
vs.fields.mf_vcodec = ProtoField.new("Video codec", "vs.mf.vcodec", ftypes.UINT32, mf_vcodec_desc, base.HEX)
vs.fields.mf_arate = ProtoField.new("Audio sample rate", "vs.mf.arate", ftypes.UINT32)
local mf_acodec_desc = {
	[0x0001] = "PCM",
	[0x0006] = "G.711 a-law",
	[0x0007] = "G.711 mu-law",
	[0x0031] = "GSM",
	[0x0042] = "G.723",
	[0x0041] = "G.728",
	[0x0083] = "G.729 Annex A",
	[0x0065] = "G.722",
	[0x7001] = "G.722.1 24kbit/s",
	[0x7002] = "G.722.1 32kbit/s",
	[0x7003] = "Speex",
	[0x7004] = "iSAC",
	[0x7005] = "G.722.1 Annex C 24kbit/s",
	[0x7006] = "G.722.1 Annex C 32kbit/s",
	[0x7007] = "G.722.1 Annex C 48kbit/s",
	[0x7ff0] = "Opus",
	[0x0055] = "MP3",
	[0xa106] = "AAC",
}
vs.fields.mf_acodec = ProtoField.new("Audio codec", "vs.mf.acodec", ftypes.UINT32, mf_acodec_desc, base.HEX)
vs.fields.mf_abuf = ProtoField.new("Audio buffer length", "vs.mf.abuf", ftypes.UINT32)
vs.fields.mf_vfps = ProtoField.new("Video FPS", "vs.mf.vfps", ftypes.UINT32)
vs.fields.mf_vstereo = ProtoField.new("Stereo", "vs.mf.vstereo", ftypes.UINT32, nil, base.HEX)
vs.fields.mf_vscv = ProtoField.new("SVC mode", "vs.mf.vsvc", ftypes.UINT32, nil, base.HEX)
local mf_vhwcodec_desc = {
	[0] = "Software",
	[1] = "Logitech",
	[2] = "Intel",
	[3] = "NVidia",
}
vs.fields.mf_vhwcodec = ProtoField.new("Video HW codec", "vs.mf.vhwcodec", ftypes.UINT32, mf_vhwcodec_desc, base.HEX)

-- Broker statistics (stream::StreamStatistics)
-- TODO: Supply human readable names for fields
vs.fields.bs = ProtoField.new("Broker statictics", "vs.bs", ftypes.BYTES)
vs.fields.bs_all_frames_buffer = ProtoField.new("allFramesBuffer", "vs.bs.all_frames_buffer", ftypes.UINT16)
vs.fields.bs_all_write_bytes = ProtoField.new("allWriteBytesBand", "vs.bs.all_write_bytes", ftypes.UINT32)
vs.fields.bs_all_write_frames = ProtoField.new("allWriteFramesBand", "vs.bs.all_write_frames", ftypes.UINT32)
vs.fields.bs_ntracks = ProtoField.new("ntracks", "vs.bs.ntracks", ftypes.UINT8)
vs.fields.bs_ts = ProtoField.new("Track statictics", "vs.bs.ts", ftypes.BYTES)
vs.fields.bs_ts_track = ProtoField.new("track", "vs.bs.ts.track", ftypes.UINT8)
vs.fields.bs_ts_frames_buffer = ProtoField.new("nFramesBuffer", "vs.bs.ts.frames_buffer", ftypes.UINT16)
vs.fields.bs_ts_write_bytes = ProtoField.new("writeBytesBand", "vs.bs.ts.write_bytes", ftypes.UINT32)
vs.fields.bs_ts_write_frames = ProtoField.new("writeFramesBand", "vs.bs.ts.write_frames", ftypes.UINT32)

vs.prefs.sanity_len = Pref.bool("Sanity checks for message length", true, "If set dissector will discard messages with unusually big message lengths.")
vs.prefs.sanity_cksum = Pref.bool("Sanity checks for checksums", true, "If set dissector will discard messages if checksum validation fails.")
vs.prefs.expect_svc = Pref.bool("Expect SVC header in video frames", false, "If set dissector will try to decode SVC header from video frames.")
vs.prefs.decode_vcodec = Pref.bool("Try to decode video frame payload", false, "If set dissector will try to decode data from video frames as H264 or VP8. (Currently works only for the first slice of a video frame)")

local streams_info = {} -- indexed by stream id ("tcp.stream" field)
local vframes_info = {} -- indexed by packet number (pinfo.number)

function vs.init()
	streams_info = {}
	vframes_info = {}
end

local tcp_stream_f = Field.new("tcp.stream")
local vs_f_data_f = Field.new("vs.f.data")
local vs_f_track_f = Field.new("vs.f.track")

local h264 = Dissector.get("h264")
local vp8 = Dissector.get("vp8")

local function find_h264_start_code(tvbr)
	local size = tvbr:len()
	local off = 0
	while off < size-4 do
		if tvbr:range(off+0,1):uint() == 0 then
			if tvbr:range(off+1,1):uint() == 0 then
				if tvbr:range(off+2,1):uint() == 0 then
					if tvbr:range(off+3,1):uint() == 0 then
						-- Strange case
						off = off + 1
					elseif tvbr:range(off+3,1):uint() == 1 then
						return off, 4
					else
						off = off + 4
					end
				elseif tvbr:range(off+2,1):uint() == 1 then
					return off, 3
				else
					off = off + 3
				end
			else
				off = off + 2
			end
		else
			off = off + 1
		end
	end
end

local function get_h264_nal_from_bytestream(tvbr)
	local size = tvbr:len()
	local sc_size
	if size >= 3 and tvbr:range(0,3):uint() == 1 then
		sc_size = 3
	elseif size >= 4 and tvbr:range(0,4):uint() == 1 then
		sc_size = 4
	else
		return
	end

	local result, next_sc_size = find_h264_start_code(tvbr:range(sc_size))
	if result then
		debug("got NAL: begin="..sc_size..", end="..(sc_size+result))
		return tvbr:range(sc_size,result), sc_size
	else
		debug("got NAL: begin="..sc_size)
		return tvbr:range(sc_size), sc_size
	end
end

local function get_handshake_size(tvbuf, offset)
	local size = tvbuf:len() - offset
	if size < 20 then return end
	return 20 + bit32.extract(tvbuf(offset+16,4):le_uint(), 19, 13) + 1;
end

local function dissect_handshake_header(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	local hs_tree = root:add(vs.fields.hs, tvbuf(offset, hs_size))
	hs_tree:add(vs.fields.hs_primary, tvbuf(offset+0,16), tvbuf(offset+0,16):stringz())

	local version = bit32.extract(tvbuf(offset+16,4):le_uint(), 0, 5)
	hs_tree:add_le(vs.fields.hs_version, tvbuf(offset+16,4))
	if version ~= 1 then
		hs_tree:add_tvb_expert_info(vs.experts.hs_version_unk, tvbuf(offset+16,4))
	end

	local head_cksum = bit32.extract(tvbuf(offset+16,4):le_uint(), 5, 8)
	hs_tree:add_le(vs.fields.hs_head_cksum, tvbuf(offset+16,4))
	local body_cksum = bit32.extract(tvbuf(offset+16,4):le_uint(), 13, 6)
	hs_tree:add_le(vs.fields.hs_body_cksum, tvbuf(offset+16,4))
	local body_length = bit32.extract(tvbuf(offset+16,4):le_uint(), 19, 13)
	hs_tree:add_le(vs.fields.hs_body_length, tvbuf(offset+16,4))

	local real_head_cksum = 0xac + version + body_cksum + body_length
	for i = 0, 15 do
		real_head_cksum = real_head_cksum + tvbuf(offset+i,1):le_uint()
	end
	real_head_cksum = real_head_cksum % 256
	if head_cksum ~= real_head_cksum then
		hs_tree:add_proto_expert_info(vs.experts.hs_head_cksum_bad)
		hs_tree:add_le(vs.fields.hs_real_head_cksum, real_head_cksum):set_generated()
	end

	local real_body_cksum = 0xca
	for i = 20, 20 + body_length, 3 do
		real_body_cksum = real_body_cksum + tvbuf(offset+i,1):le_uint() - i
	end
	real_body_cksum = real_body_cksum % 64
	if body_cksum ~= real_body_cksum then
		hs_tree:add_proto_expert_info(vs.experts.hs_body_cksum_bad)
		hs_tree:add_le(vs.fields.hs_real_body_cksum, real_body_cksum):set_generated()
	end

	return hs_tree
end

local function is_stream_handshake_response(tvbuf, offset, body_size)
	-- Stream handshake response contains only optional 'tracks' which is 32 bytes long if present
	-- and if it is not nTracks == 255
	local ntracks = tvbuf(offset+20+1,1):le_uint()
	if ntracks == 255 then
		return body_size == 2
	else
		return body_size == 2+32
	end
end

local function dissect_tracks(tvbuf, pktinfo, root, offset)
	local ntracks = tvbuf(offset+0,1):le_uint()
	if ntracks ~= 255 then
		root:add(vs.fields.hs_ntracks, tvbuf(offset+0,1))
		local tracks = {}
		for i = 0, 31 do
			local b = tvbuf(offset+1+i,1):le_uint()
			for j = 0, 7 do
				if bit32.btest(b,1) then
					table.insert(tracks, 8*i+j)
				end
				b = bit32.rshift(b,1)
			end
		end
		root:add(vs.fields.hs_tracks, tvbuf(offset+1,32)):set_text("Tracks: "..table.concat(tracks, ", "))
		return offset + 1 + 32
	else
		root:add(vs.fields.hs_ntracks, tvbuf(offset+0,1)):append_text(" (no tracks present)")
		return offset + 1
	end
end

local function dissect_stream_handshake(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	local hs_size = get_handshake_size(tvbuf, offset)
	if not hs_size then
		return -DESEGMENT_ONE_MORE_SEGMENT
	elseif size < hs_size then
		return -(hs_size-size)
	end
	local body_size = hs_size - 20

	pktinfo.cols.protocol:set("Visicron")
	local tree = root:add(vs, tvbuf(offset, hs_size))
	local hs_tree = dissect_handshake_header(tvbuf, pktinfo, tree, offset)

	local off = offset + 20
	if is_stream_handshake_response(tvbuf, offset, body_size) then
		local cl_type = tvbuf(off,1):le_uint()
		hs_tree:add(vs.fields.hs_cl_type, tvbuf(off,1))
		if not cl_type == 1 then
			hs_tree:add_proto_expert_info(vs.experts.hs_cl_type_unk)
		end
		off = off + 1

		off = dissect_tracks(tvbuf, pktinfo, hs_tree, off)

        pktinfo.cols.info:set("Stream handshake response")
	else
		local cl_type = tvbuf(off,1):le_uint()
		hs_tree:add(vs.fields.hs_cl_type, tvbuf(off,1))
		if not (cl_type == 0 or cl_type == 1) then
			hs_tree:add_proto_expert_info(vs.experts.hs_cl_type_unk)
		end
		off = off + 1

		local conf_name_size = tvbuf(off,1):le_uint()
		off = off + 1
		hs_tree:add(vs.fields.hs_conf, tvbuf(off,conf_name_size+1))
		off = off + conf_name_size + 1

		local part_name_size = tvbuf(off,1):le_uint()
		off = off + 1
		hs_tree:add(vs.fields.hs_part, tvbuf(off,part_name_size+1))
		off = off + part_name_size + 1

		local conn_part_name_size = tvbuf(off,1):le_uint()
		off = off + 1
		hs_tree:add(vs.fields.hs_conn_part, tvbuf(off,conn_part_name_size+1))
		off = off + conn_part_name_size + 1

		local ep_name_size = tvbuf(off,1):le_uint()
		off = off + 1
		hs_tree:add(vs.fields.hs_ep, tvbuf(off,ep_name_size+1))
		off = off + ep_name_size + 1

		off = dissect_tracks(tvbuf, pktinfo, hs_tree, off)

        pktinfo.cols.info:set("Stream handshake")
	end

	if off < offset + hs_size then
		hs_tree:add_tvb_expert_info(vs.experts.hs_body_unk, tvbuf(off,offset+hs_size-off))
	end

	return hs_size
end

local function dissect_generic_handshake(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	local hs_size = get_handshake_size(tvbuf, offset)
	if not hs_size then
		return -DESEGMENT_ONE_MORE_SEGMENT
	elseif size < hs_size then
		return -(hs_size-size)
	end
	local body_size = hs_size - 20

	pktinfo.cols.protocol:set("Visicron")
	local tree = root:add(vs, tvbuf(offset, hs_size))
	local hs_tree = dissect_handshake_header(tvbuf, pktinfo, tree, offset)

	pktinfo.cols.info:set("Handshake")

	hs_tree:add_tvb_expert_info(vs.experts.hs_body_unk, tvbuf(offset+20,body_size))

	return hs_size
end

local function dissect_handshake(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	if size < 16 then
		return -DESEGMENT_ONE_MORE_SEGMENT
	end

	local primary_field = tvbuf(offset,16):stringz()
	debug("Found handshake primary_field: \""..primary_field.."\"")
	if primary_field == "_VS_STREAMS_" then
		return dissect_stream_handshake(tvbuf, pktinfo, root, offset)
	else
		return dissect_generic_handshake(tvbuf, pktinfo, root, offset)
	end
end

local function dissect_media_format(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	if size < 4 then
		-- Need at least 4 bytes to get VS_MediaFormat::dwSize
		return -DESEGMENT_ONE_MORE_SEGMENT
	end
	local mf_size = tvbuf(offset+0,4):le_uint()
	if size < mf_size then
		return -(mf_size-size)
	end

	if mf_size < 24 then
		return 0
	end

	local mf_tree = root:add(vs.fields.mf, tvbuf(offset, mf_size))
	mf_tree:add_le(vs.fields.mf_length, tvbuf(offset+0,4))
	mf_tree:add_le(vs.fields.mf_vheigth, tvbuf(offset+4,4))
	mf_tree:add_le(vs.fields.mf_vwidth, tvbuf(offset+8,4))
	mf_tree:add_le(vs.fields.mf_vcodec, tvbuf(offset+12,4))
	mf_tree:add_le(vs.fields.mf_arate, tvbuf(offset+16,4))
	mf_tree:add_le(vs.fields.mf_acodec, tvbuf(offset+20,4))
	if mf_size >= 28 then
		mf_tree:add_le(vs.fields.mf_abuf, tvbuf(offset+24,4))
	end
	if mf_size >= 32 then
		mf_tree:add_le(vs.fields.mf_vfps, tvbuf(offset+28,4))
	end
	if mf_size >= 36 then
		mf_tree:add_le(vs.fields.mf_vstereo, tvbuf(offset+32,4))
	end
	if mf_size >= 40 then
		mf_tree:add_le(vs.fields.mf_vscv, tvbuf(offset+36,4))
	end
	if mf_size >= 44 then
		mf_tree:add_le(vs.fields.mf_vhwcodec, tvbuf(offset+40,4))
	end

	return mf_size
end

local function dissect_broker_stats(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	if size < 11 then
		-- Need at least 11 bytes to get stream::StreamStatistics::ntracks
		return -DESEGMENT_ONE_MORE_SEGMENT
	end
	local ntracks = tvbuf(offset+10,1):le_uint()
	local bs_size = 11+11*ntracks
	if size < bs_size then
		return -(bs_size-size)
	end

	local bs_tree = root:add(vs.fields.bs, tvbuf(offset,bs_size))
	bs_tree:add_le(vs.fields.bs_all_frames_buffer, tvbuf(offset+0,2))
	bs_tree:add_le(vs.fields.bs_all_write_bytes, tvbuf(offset+2,4))
	bs_tree:add_le(vs.fields.bs_all_write_frames, tvbuf(offset+6,4))
	bs_tree:add_le(vs.fields.bs_ntracks, tvbuf(offset+10,1))

	for i = 0, ntracks-1 do
		local ts_offset = offset+11+11*i
		local ts_tree = bs_tree:add(vs.fields.bs_ts, tvbuf(ts_offset,11))
		ts_tree:add_le(vs.fields.bs_ts_track, tvbuf(ts_offset+0,1))
		ts_tree:add_le(vs.fields.bs_ts_frames_buffer, tvbuf(ts_offset+1,2))
		ts_tree:add_le(vs.fields.bs_ts_write_bytes, tvbuf(ts_offset+3,4))
		ts_tree:add_le(vs.fields.bs_ts_write_frames, tvbuf(ts_offset+7,4))
	end

	return bs_size
end

local function dissect_frame(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	if size < 2 then
		-- Need at least 2 bytes to get stream::FrameHeader::length
		return -DESEGMENT_ONE_MORE_SEGMENT
	end

	local payload_size = tvbuf(offset, 2):le_uint()
	if (payload_size > 4004 and vs.prefs.sanity_len) then
		return 0
	end
	local frame_size = 8 + payload_size
	if size < frame_size then
		return -(frame_size-size)
	end

	local track = tvbuf(offset+6,1):le_uint()
	local cksum = tvbuf(offset+7,1):le_uint()
	local real_cksum = 0xac
	for i = 8, 8 + payload_size-1, 0xea do
		real_cksum = real_cksum + tvbuf(offset+i,1):le_uint()
	end
	real_cksum = real_cksum % 256
	if cksum ~= real_cksum and vs.prefs.sanity_cksum then
		return 0
	end

	pktinfo.cols.protocol:set("Visicron")
	local tree = root:add(vs, tvbuf(offset, frame_size))
	local frame_tree = tree:add(vs.fields.f, tvbuf(offset, frame_size))
	frame_tree:add_le(vs.fields.f_length, tvbuf(offset+0,2))
	frame_tree:add_le(vs.fields.f_ticks, tvbuf(offset+2,4))
	frame_tree:add_le(vs.fields.f_track, tvbuf(offset+6,1))
	if not (track == 1 or track == 2 or track == 254 or track == 255) then
		frame_tree:add_proto_expert_info(vs.experts.f_track_unk)
	end
	frame_tree:add_le(vs.fields.f_cksum, tvbuf(offset+7,1))
	if cksum ~= real_cksum then
		frame_tree:add_proto_expert_info(vs.experts.f_cksum_bad)
		frame_tree:add_le(vs.fields.f_real_cksum, real_cksum):set_generated()
	end

	if track == 1 then
		frame_tree:add(vs.fields.f_data, tvbuf(offset+8, payload_size))

		pktinfo.cols.info:set("Audio frame: len="..payload_size)
	elseif track == 2 then
		local svc_size = 0
		if vs.prefs.expect_svc then
			-- stream::SVCHeader
			svc_size = 1
			local svc_offset = offset + 8 + (payload_size - svc_size)

			local svc_tree = frame_tree:add(vs.fields.svc, tvbuf(svc_offset+0,1))
			svc_tree:add(vs.fields.svc_maxspatial, tvbuf(svc_offset+0,1))
			svc_tree:add(vs.fields.svc_temporal, tvbuf(svc_offset+0,1))
			svc_tree:add(vs.fields.svc_spatial, tvbuf(svc_offset+0,1))
			svc_tree:add(vs.fields.svc_quality, tvbuf(svc_offset+0,1))
		end

		-- stream::SliceHeader
		local vh_size = 3
		local vh_offset = offset + 8 + (payload_size - vh_size - svc_size)

		local slice = tvbuf(vh_offset+0,1):le_uint()
		frame_tree:add_le(vs.fields.f_vh_slice, tvbuf(vh_offset+0,1))
		local slice_max = tvbuf(vh_offset+1,1):le_uint()
		frame_tree:add_le(vs.fields.f_vh_slice_max, tvbuf(vh_offset+1,1))
		local frame_cnt = tvbuf(vh_offset+2,1):le_uint()
		frame_tree:add_le(vs.fields.f_vh_frame_cnt, tvbuf(vh_offset+2,1))

		if slice > slice_max then
			frame_tree:add_proto_expert_info(vs.experts.f_vh_slice_bad)
		end

		local is_key = false
		local data_size
		local data_offset
		if slice == slice_max then
			-- Metadata from first slice
			is_key = tvbuf(offset+8,1):le_uint() > 0
			frame_tree:add(vs.fields.f_key, tvbuf(offset+8,1), is_key)
			frame_tree:add_le(vs.fields.f_duration, tvbuf(offset+9,4))
			data_size = payload_size - 5 - vh_size - svc_size
			data_offset = offset + 13

			local data_tree = frame_tree:add(vs.fields.f_data, tvbuf(data_offset, data_size))
			if vs.prefs.decode_vcodec then
				local off = data_offset
				if h264 then
					while true do
						debug("trying to find NAL: off="..off..", sz="..(data_size-(off-data_offset)))
						local nal_range, sc_size = get_h264_nal_from_bytestream(tvbuf(off,data_size-(off-data_offset)))
						if nal_range then
							h264:call(nal_range:tvb(), pktinfo, data_tree)
							off = off + sc_size + nal_range:len()
						else
							break
						end
					end
				end
				if vp8 and off == data_offset then
					vp8:call(tvbuf(data_offset,data_size):tvb(), pktinfo, data_tree)
				end
			end
		else
			data_size = payload_size - vh_size - svc_size
			data_offset = offset + 8
			frame_tree:add(vs.fields.f_data, tvbuf(data_offset, data_size))
		end

		local s_id = tcp_stream_f().value
		local vf_id = pktinfo.number
		if s_id and vf_id then
			if not streams_info[s_id] then
				streams_info[s_id] = {}
			end
			if not vframes_info[vf_id] then
				vframes_info[vf_id] = {
					slice = slice,
					frame_cnt = frame_cnt,
					prev_vf_id = streams_info[s_id].prev_vf_id,
				}
			end
			local prev_vf_id = vframes_info[vf_id].prev_vf_id
			local prev_vf = prev_vf_id and vframes_info[prev_vf_id]
			if prev_vf then
				local last_frame_cnt = streams_info[s_id].last_frame_cnt
				local last_slice = streams_info[s_id].last_slice
				debug("f:"..frame_cnt..", pf:"..prev_vf.frame_cnt..", s:"..slice..", ps:"..prev_vf.slice)
				if frame_cnt == prev_vf.frame_cnt then
					if slice ~= prev_vf.slice-1 then
						frame_tree:add_proto_expert_info(vs.experts.f_slice_missed)
					end
				elseif frame_cnt == (prev_vf.frame_cnt+1)%256 then
					if prev_vf.slice ~= 0 then
						frame_tree:add_proto_expert_info(vs.experts.f_slice_missed_pf)
					end
				else
					frame_tree:add_proto_expert_info(vs.experts.f_frame_missed)
				end
			end
			if not pktinfo.visited then
				-- First pass
				streams_info[s_id].prev_vf_id = vf_id
			end
		end

		local key_tag = is_key and ", key" or ""
		local slice_tag = slice_max > 0 and (" slice "..slice.."/"..slice_max) or ""
		pktinfo.cols.info:set("Video frame"..slice_tag..": len="..data_size.. ", cnt="..frame_cnt..key_tag)
	elseif track == 254 then
		local cmd_tree = frame_tree:add(vs.fields.cmd, tvbuf(offset+8, payload_size))
		local cmd_type = tvbuf(offset+8,1):le_uint()
		cmd_tree:add_le(vs.fields.cmd_type, tvbuf(offset+8,1))
		local cmd_subtype = tvbuf(offset+9,1):le_uint()
		cmd_tree:add_le(vs.fields.cmd_subtype, tvbuf(offset+9,1))
		local cmd_ret = tvbuf(offset+10,1):le_uint()
		cmd_tree:add_le(vs.fields.cmd_ret, tvbuf(offset+10,1))
		cmd_tree:add_le(vs.fields.cmd_data_length, tvbuf(offset+11,1))

		local data_size = tvbuf(offset+11,1):le_uint()
		local data_offset = offset + 12
		if payload_size < 4 + data_size then
			cmd_tree:add_proto_expert_info(vs.experts.cmd_truncated)
			data_size = payload_size - 4
		end
		if cmd_type == 2 then
			if data_size >= 4 then
				cmd_tree:add_le(vs.fields.cmd_bitrate, tvbuf(data_offset+0,4))
			end
			if data_size > 4 then
				cmd_tree:add_tvb_expert_info(vs.experts.cmd_data_unk, tvbuf(data_offset+4,data_size-4))
			end
		elseif cmd_type == 4 or cmd_type == 5 then
			local result = dissect_media_format(tvbuf, pktinfo, cmd_tree, data_offset)
			if result <= 0 then
				cmd_tree:add_proto_expert_info(vs.experts.cmd_truncated)
				cmd_tree:add(vs.fields.cmd_data, tvbuf(data_offset,data_size))
			elseif data_size > result then
				cmd_tree:add_tvb_expert_info(vs.experts.cmd_data_unk, tvbuf(data_offset+result,data_size-result))
			end
		elseif cmd_type == 10 then
			local result = dissect_broker_stats(tvbuf, pktinfo, cmd_tree, data_offset)
			if result <= 0 then
				cmd_tree:add_proto_expert_info(vs.experts.cmd_truncated)
				cmd_tree:add(vs.fields.cmd_data, tvbuf(data_offset,data_size))
			elseif data_size > result then
				cmd_tree:add_tvb_expert_info(vs.experts.cmd_data_unk, tvbuf(data_offset+result,data_size-result))
			end
		elseif cmd_type == 11 then
			if data_size >= 4 then
				cmd_tree:add_le(vs.fields.cmd_bitrate, tvbuf(data_offset+0,4))
			end
			if data_size >= 8 then
				cmd_tree:add_le(vs.fields.cmd_vbitrate, tvbuf(data_offset+4,4))
			end
			if data_size > 8 then
				cmd_tree:add_tvb_expert_info(vs.experts.cmd_data_unk, tvbuf(data_offset+8,data_size-8))
			end
		else
			if data_size > 0 then
				cmd_tree:add(vs.fields.cmd_data, tvbuf(data_offset, data_size))
			end
		end

		if payload_size > 4 + data_size then
			cmd_tree:add_tvb_expert_info(vs.experts.cmd_data_unk, tvbuf(data_offset+data_size, payload_size-(4+data_size)))
		end

		local pkt_info = "Command: "..(cmd_type_desc[cmd_type] or "unknown")
		pkt_info = pkt_info.." ("..(cmd_subtype_desc[cmd_subtype] or "unknown")
		if cmd_subtype == 2 then
			pkt_info = pkt_info..": "..(cmd_ret_desc[cmd_ret] or "unknown")
		end
		pkt_info = pkt_info..")"
		pktinfo.cols.info:set(pkt_info)
	elseif track == 255 then
		if payload_size == 0 then
			pktinfo.cols.info:set("Ping")
		elseif payload_size == 1 then
			pktinfo.cols.info:set("Key frame request")
		else
			pktinfo.cols.info:set("Broker statistics")
			local result = dissect_broker_stats(tvbuf, pktinfo, frame_tree, offset+8)
			if result <= 0 then
				frame_tree:add_proto_expert_info(vs.experts.cmd_truncated)
				frame_tree:add(vs.fields.cmd_data, tvbuf(offset+8,payload_size))
			elseif payload_size > result then
				frame_tree:add_tvb_expert_info(vs.experts.cmd_data_unk, tvbuf(offset+8+result,payload_size-result))
			end
		end
	end
	return frame_size
end

local function dissect_one(tvbuf, pktinfo, root, offset)
	local size = tvbuf:len() - offset

	if size < 4 then
		-- Need at least 4 bytes to check for handshake primary field
		return -DESEGMENT_ONE_MORE_SEGMENT
	end

	if tvbuf(offset,4):string() == "_VS_" then
		return dissect_handshake(tvbuf, pktinfo, root, offset)
	else
		return dissect_frame(tvbuf, pktinfo, root, offset)
	end
end

function vs.dissector(tvbuf, pktinfo, root)
	local size = tvbuf:len()

	local consumed = 0
	while consumed < size do
		local result = dissect_one(tvbuf, pktinfo, root, consumed)
		if result > 0 then
			consumed = consumed + result
		elseif result < 0 then
			if size ~= tvbuf:reported_len() then
				return 0
			end
			pktinfo.desegment_offset = consumed
			pktinfo.desegment_len = -result
			return size
		else
			return 0
		end
	end
	return consumed
end

DissectorTable.get("tcp.port"):add(4307, vs)

function dump_vs_streams(dir, fn_prefix)
	if not fn_prefix or fn_prefix == "" then
		fn_prefix = "dump"
	end

	local td = {}
	td.streams = {}

	local filter = "vs.f && !tcp.analysis.retransmission"
	if gui_enabled() then
		local global_filter = get_filter()
		if global_filter and global_filter ~= "" then
			filter = filter .. " && (" .. global_filter .. ")"
		end
	end
	print("Using filter \"" .. filter .. "\"")

	tap = Listener.new(nil, filter)
	if not tap then
		return
	end

	function tap.packet(pktinfo, tvbuf)
		local t_id = vs_f_track_f().value
		if t_id ~= 1 and t_id ~= 2 then
			return
		end

		local s_id = tcp_stream_f().value
		local stream = td.streams[s_id]
		if not stream then
			stream = {
				tracks = {}
			}
			td.streams[s_id] = stream
		end

		local track = stream.tracks[t_id]
		if not track then
			track = {}

			local reason
			local fn = dir .. "/" .. fn_prefix .. ".stream-" .. s_id .. ".track-" .. t_id .. ".raw"
			track.file, reason = io.open(fn, "wb")
			if track.file then
				track.file:setvbuf("full")
			else
				critical("io.open(\"" .. fn .. "\") failed: " .. reason)
			end

			stream.tracks[t_id] = track
		end

		if track.file then
			local data = vs_f_data_f()
			local result, reason = track.file:write(data.value:raw())
			if not result then
				critical("file:write failed: " .. reason)
			end
		end
	end

	function each_file(f)
		for _, stream in pairs(td.streams) do
			for _, track in pairs(stream.tracks) do
				if track.file then
					f(track.file)
				end
			end
		end
	end

	function tap.draw()
		each_file(function (file) file:flush() end)
	end

	function tap.reset()
		each_file(function (file) file:close() end)
		td.streams = {}
	end

	print("VS stream tap: start")
	retap_packets()
	print("VS stream tap: end")
	tap:remove()
	each_file(function (file) file:close() end)
end

function vs_stream_dump_menu()
	function on_ok(dir, fn_prefix)
		if not dir or dir == "" then
			local homedir = os.getenv("HOME") or os.getenv("USERPROFILE")
			if homedir and homedir ~= "" then
				dir = homedir .. "/Documents"
			end
		end
		if not dir or dir == "" then
			return
		end
		print("Will save dumps to directory \"" .. dir .. "\"")

		dump_vs_streams(dir, fn_prefix)
	end

	new_dialog("Visicron stream dump options", on_ok, "Directory (default is \"Documents\" folder)", "Filename prefix (default: \"dump\")")
end

register_menu("Visicron stream dump", vs_stream_dump_menu, MENU_TOOLS_UNSORTED)
