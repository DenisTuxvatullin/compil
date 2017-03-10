#pragma once
#include <string>
#include <stack>

#define EOS		(-1)

class InputStream
{
public:
	// Создает входной поток на основе строки
	InputStream(const std::string &text, int lineNumber);

	// Возвращает позицию в потоке
	size_t Tell() const;

	// Устанавливает позицию
	// Возвращает позицию которую удалось установить
	size_t Seek(size_t pos);

	// Продвигает позицию вперед на shift символов
	// Возвращает кол-во символов на которые удалось продвинуть
	size_t Forward(size_t shift = 1);

	// Пропускает пробелы, табуляции и т.п.
	// Возвращает кол-во пропущенных символов
	size_t SkipSpaces();

	// Возвращает true если достигнут конец потока
	bool IsEOS() const;

	// Возвращает length символов с текущей позиции
	std::string Get(size_t length) const;

	// Возвращает true если текст в текущей позиции InputStream
	// полностю совпадает с текстом в параметре text
	bool Match(const char *text) const;

	// Если текст совпадает, то перейти на позицию после
	// совпавшего символа
	bool InputStream::MatchF(const char *text);

	// Возвращает оставшийся текст
	const char *GetRemainingText() const;

	// Знак в текущей позиции
	int GetChar() const;

	// Сохранить позицию
	void PushPosition();
	// Вытащить из стека последнюю сохраненную позицию, не восстанавливая её
	size_t PopPosition();
	// Восстановить последнюю сохраненную позицию
	size_t RestorePosition();

	// Возвращает номер строки в исходном файле
	int GetLineNumber() const;

	// Возвращает полное содержимое строки,
	// на основе которой был создан поток
	const char *GetLine() const;

private:
	const std::string m_data;

	// Текущая позиция
	std::string::const_iterator m_pos;

	// Стек, в котором хранятся предыдущие позиции
	std::stack<size_t> m_pos_stack;

	// Номер строки в исходном файле
	int m_lineNumber;
};
