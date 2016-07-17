<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka user
 *   */
class ZamereniRepository extends Repository
{
	public function getNamesAsArray()
	{
		$tmp = $this->getTable();
		return $tmp->fetchPairs('Nazev_zamereni', 'Nazev_zamereni');
	}

	public function findByName($nazev_zamereni)
	{
		return $this->findAll()->where('Nazev_zamereni', $nazev_zamereni)->fetch();
	}

	public function addZamereni($values)
	{
		$this->getTable()->insert(array(
			'Nazev_zamereni' => $values->nazev_zamereni,
			'Popis_zamereni' => $values->popis_zamereni,
			'Uplatneni' => $values->uplatneni
		));
	}

	public function deleteZamereni($nazev_zamereni) {
		$this->findBy(array('Nazev_zamereni' => $nazev_zamereni))->delete();
	}

}
