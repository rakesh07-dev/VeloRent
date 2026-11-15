// Exceptions.h  -- Custom exception hierarchy for VeloRent
// Owner: Tianshu Aswal (Module 4)  |  Used by every module
#ifndef VELORENT_EXCEPTIONS_H
#define VELORENT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

// Base class: everything VeloRent throws derives from this, so main()
// can catch (const VeloException&) once and print a clean message.
class VeloException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class DuplicateKeyException : public VeloException {   // e.g. same VIN added twice
public:
    using VeloException::VeloException;
};

class NotFoundException : public VeloException {       // e.g. VIN / hub / booking not found
public:
    using VeloException::VeloException;
};

class InvalidInputException : public VeloException {   // e.g. days <= 0, negative rate
public:
    using VeloException::VeloException;
};

class FileIOException : public VeloException {         // e.g. ledger.bin cannot be opened
public:
    using VeloException::VeloException;
};

class GraphException : public VeloException {          // e.g. negative route cost
public:
    using VeloException::VeloException;
};

#endif
