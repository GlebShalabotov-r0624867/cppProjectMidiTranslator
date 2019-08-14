#include "midi.h"
#include "../io/read.h"
#include "../io/endianness.h"
#include "../io/vli.h"

namespace midi {

	void read_chunk_header(std::istream& in, CHUNK_HEADER* header) {
		io::read_to(in, header);
		io::switch_endianness(&header->size);
	}

	std::string header_id(CHUNK_HEADER header) {
		std::string out = "";
		for (char c : header.id) {
			out += c;
		}
		return out;
	}

	void read_mthd(std::istream& ss, MTHD* mthd) {
		io::read_to(ss, mthd);
		io::switch_endianness(&mthd->header.size);
		io::switch_endianness(&mthd->type);
		io::switch_endianness(&mthd->ntracks);
		io::switch_endianness(&mthd->division);
	}

	bool is_sysex_event(uint8_t byte) {
		return (byte == 0xF0) | (byte == 0xF7);
	}

	bool is_meta_event(uint8_t byte) {
		return (byte == 0xFF);
	}

	bool is_midi_event(uint8_t byte) {
		return (byte >= 0x80) & (byte < 0xF0);
	}

	bool is_running_status(uint8_t byte) {
		return byte < 0x80;
	}

	uint8_t extract_midi_event_type(uint8_t status) {
		return status >> 4;
	}

	Channel extract_midi_event_channel(uint8_t status) {
		return Channel(status & 0x0F); 
	}


	bool is_note_off(uint8_t status) {
		return (status == 0x08);
	}

	bool is_note_on(uint8_t status) {
		return (status == 0x09);
	}

	bool is_polyphonic_key_pressure(uint8_t status) {
		return (status == 0x0A);
	}

	bool is_control_change(uint8_t status) {
		return (status == 0x0B);
	}

	bool is_program_change(uint8_t status) {
		return (status == 0x0C);
	}

	bool is_channel_pressure(uint8_t status) {
		return (status == 0x0D);
	}

	bool is_pitch_wheel_change(uint8_t status) {
		return (status == 0x0E); 
	}


	void read_mtrk(std::istream& in, EventReceiver& receiver) {


		CHUNK_HEADER header;
		read_chunk_header(in, &header);
		bool end = false;
		uint8_t prevID;

		while (!end){
			Duration duration(io::read_variable_length_integer(in));
			uint8_t iD = io::read<uint8_t>(in);
			uint8_t firstBytes;

			if (is_running_status(iD)){
				firstBytes = iD;
				iD = prevID;
			}

			else{
				firstBytes = io::read<uint8_t>(in);
			}

			if (is_meta_event(iD)){
				uint8_t type = firstBytes;
				uint64_t length = io::read_variable_length_integer(in);
				std::unique_ptr<uint8_t[]> data =
				io::read_array<uint8_t>(in, length);

				receiver.meta(duration, type, std::move(data), length);

				if (type == 0x2F) {
					end= true;
				}

			}

			else if (is_sysex_event(iD)){
				in.putback(firstBytes);
				auto length = io::read_variable_length_integer(in);
				std::unique_ptr<uint8_t[]> data =
					io::read_array<uint8_t>(in, length);
				receiver.sysex(duration, std::move(data), length);
			}
			else if (is_midi_event(iD)){

				uint8_t midiEventType = extract_midi_event_type(iD);
				Channel channel(extract_midi_event_channel(iD));

				if (is_note_off(midiEventType) || is_note_on(midiEventType)){

					NoteNumber note = NoteNumber(firstBytes);
					uint8_t velo = io::read<uint8_t>(in);

					if (is_note_off(midiEventType)) {
						receiver.note_off(duration, channel, note, velo);
					}

					else {
						receiver.note_on(duration, channel, note, velo);
					}
				}

				else if (is_polyphonic_key_pressure(midiEventType)){
					NoteNumber note(firstBytes);
					uint8_t pressure = io::read<uint8_t>(in);
					receiver.polyphonic_key_pressure(duration, channel,
						note, pressure);
				}

				else if (is_control_change(midiEventType)){
					uint8_t controller = firstBytes;
					uint8_t value = io::read<uint8_t>(in);
					receiver.control_change(duration, channel,
						controller, value);
				}
				else if (is_program_change(midiEventType))
				{
					Instrument program(firstBytes);
					receiver.program_change(duration, channel, program);
				}
				else if (is_channel_pressure(midiEventType))
				{
					uint8_t pressure = firstBytes;
					receiver.channel_pressure(duration, channel, pressure);
				}
				else if (is_pitch_wheel_change(midiEventType))
				{
					uint8_t lower = firstBytes;
					uint8_t upper = io::read<uint8_t>(in);
					uint16_t value = upper << 7 | lower;
					receiver.pitch_wheel_change(duration, channel, value);
				}
			}
			prevID = iD;
		}
	}

	std::ostream & operator<<(std::ostream & out, const NOTE & note)
	{
		return out << "Note(number=" <<
			note.note_number << ",start=" <<
			note.start << ",duration=" <<
			note.duration << ",instrument=" <<
			note.instrument << ")";
	}

	bool NOTE::operator==(const NOTE & other) const
	{
		if (other.note_number != this->note_number) {
			return false;
		}
		else if (other.start != this->start) {
			return false;
		}
		else if (other.velocity != this->velocity) {
			return false;
		}
		else if (other.duration != this->duration) {
			return false;
		}
		else if (other.instrument != this->instrument) {
			return false;
		}
		else {
			return true;
		}
	}

	bool NOTE::operator!=(const NOTE & other) const
	{
		return this->note_number != other.note_number ||
			this->start != other.start ||
			this->duration != other.duration ||
			this->velocity != other.velocity ||
			this->instrument != other.instrument;
	}


	//CHANNEL

	void ChannelNoteCollector::note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		if (velocity == 0) {
			this->note_off(dt, channel, note, velocity);
		}
		else {
			this->time += dt;
			if (this->channel == channel) {
				if (this->snelheden[value(note)] != 128) {
					this->note_off(Duration(0), channel, note, velocity);
				}
				this->timeArray[value(note)] = this->time;
				this->snelheden[value(note)] = velocity;
			}
		}


	}

	void ChannelNoteCollector::note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		this->time += dt;
		if (this->channel == channel) {
			NOTE n = NOTE(note, this->timeArray[value(note)],
				Duration(this->time - this->timeArray[value(note)]),
				this->snelheden[value(note)],
				this->instrument);
			this->noteReceiver(n);
			this->snelheden[value(note)] = 128;
		}
	}

	void ChannelNoteCollector::polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure)
	{
		this->time += dt;
	}

	void ChannelNoteCollector::control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value)
	{
		this->time += dt;
	}

	void ChannelNoteCollector::program_change(Duration dt, Channel channel, Instrument program)
	{
		this->time += dt;

		if (this->channel == channel) {
			this->instrument = program;
		}
	}

	void ChannelNoteCollector::channel_pressure(Duration dt, Channel channel, uint8_t pressure)
	{
		this->time += dt;

	}

	void ChannelNoteCollector::pitch_wheel_change(Duration dt, Channel channel, uint16_t value)
	{
		this->time  += dt;

	}

	void ChannelNoteCollector::meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->time  += dt;
	}

	void ChannelNoteCollector::sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->time  += dt;

	}


	//MULTIEVENT

	void EventMulticaster::note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->note_on(dt, channel, note, velocity);
		}
	}

	void EventMulticaster::note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->note_off(dt, channel, note, velocity);
		}
	}

	void EventMulticaster::polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->polyphonic_key_pressure(dt, channel, note, pressure);
		}
	}

	void EventMulticaster::control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->control_change(dt, channel, controller, value);
		}
	}

	void EventMulticaster::program_change(Duration dt, Channel channel, Instrument program)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->program_change(dt, channel, program);
		}
	}

	void EventMulticaster::channel_pressure(Duration dt, Channel channel, uint8_t pressure)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->channel_pressure(dt, channel, pressure);
		}
	}

	void EventMulticaster::pitch_wheel_change(Duration dt, Channel channel, uint16_t value)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->pitch_wheel_change(dt, channel, value);
		}
	}

	void EventMulticaster::meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->meta(dt, type, std::move(data), data_size);
		}
	}

	void EventMulticaster::sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		for (std::shared_ptr<EventReceiver> receiver : this->receivers) {
			receiver->sysex(dt, std::move(data), data_size);
		}
	}


	//NOTECOLLECTOR 

	void NoteCollector::note_on(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		this->multicaster.note_on(dt, channel, note, velocity);
	}

	void NoteCollector::note_off(Duration dt, Channel channel, NoteNumber note, uint8_t velocity)
	{
		this->multicaster.note_off(dt, channel, note, velocity);
	}

	void NoteCollector::polyphonic_key_pressure(Duration dt, Channel channel, NoteNumber note, uint8_t pressure)
	{
		this->multicaster.polyphonic_key_pressure(dt,
			channel, note, pressure);
	}

	void NoteCollector::control_change(Duration dt, Channel channel, uint8_t controller, uint8_t value)
	{
		this->multicaster.control_change(dt, channel,
			controller, value);
	}

	void NoteCollector::program_change(Duration dt, Channel channel, Instrument program)
	{
		this->multicaster.program_change(dt, channel, program);
	}

	void NoteCollector::channel_pressure(Duration dt, Channel channel, uint8_t pressure)
	{
		this->multicaster.channel_pressure(dt, channel, pressure);
	}

	void NoteCollector::pitch_wheel_change(Duration dt, Channel channel, uint16_t value)
	{
		this->multicaster.pitch_wheel_change(dt, channel, value);
	}

	void NoteCollector::meta(Duration dt, uint8_t type, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->multicaster.meta(dt, type, std::move(data), data_size);
	}

	void NoteCollector::sysex(Duration dt, std::unique_ptr<uint8_t[]> data, uint64_t data_size)
	{
		this->multicaster.sysex(dt, std::move(data), data_size);
	}


	//read notes
	std::vector<NOTE> read_notes(std::istream& in)
	{
		MTHD methhead;
		read_mthd(in, &methhead);
		std::vector<NOTE> notes;

		for (int i = 0; i < methhead.ntracks; i++)
		{
			NoteCollector collector =
				NoteCollector([&notes](const NOTE& note)
			{ notes.push_back(note); });
			read_mtrk(in, collector);
		}
		return notes;
	}
	
}